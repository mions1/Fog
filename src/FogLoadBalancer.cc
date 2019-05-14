//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include <algorithm>
#include "LoadUpdate_m.h"
#include "FogLoadBalancer.h"
#include "FogJob_m.h"
#include "FogPU.h"
#include "ProbeAnswer_m.h"
#include "ProbeQuery_m.h"

namespace fog {

    Define_Module(FogLoadBalancer);
    const int INTERNAL_JOB = 0;
    const int EXTERNAL_JOB = 1;

    int ProbeQueue::nextQueryId = 0; //Needed for using this static member

    /*
     * Implementation classes
     */
    /**
     * Add the newer probeQuery at ProbeQueue queue.
     */
    void ProbeQueue::addQuery(std::vector<int> neighs, std::map<int, ProbeQueue*> pq, int jobId, FogJob* job) {
        pq[nextQueryId] = new ProbeQueue();
        pq[nextQueryId]->queryId = nextQueryId;
        pq[nextQueryId]->jobId = jobId;
        pq[nextQueryId]->job = job;
        pq[nextQueryId]->neighs = neighs;
        pq[nextQueryId]->load.resize(neighs.size(),-1);
        nextQueryId++;
    }

    LocalLoad::LocalLoad(int nServers) {
        port.resize(nServers, 0);
        load.resize(nServers, 0);
        port.reserve(nServers);
        load.reserve(nServers);
    }

    /**
     * Set new load of the pu.
     */
    void LocalLoad::updateLoad(int pu, int newLoad) {
        load.at(pu) = newLoad;
    }

    /**
     * Get pu which have the least load
     */
    int LocalLoad::getLeastLoadPU() {
        int min = load.at(0);
        int minIndex = 0;
        for (unsigned int i = 1; i < port.size(); i++) {
            if (min > load.at(i)) {
                minIndex = i;
                min = load.at(i);
            }
        }
        return minIndex;
    }

    /**
     * Get least load value.
     */
    int LocalLoad::getLeastLoad() {
        int min = load.at(0);
        for (unsigned int i = 1; i < port.size(); i++)
            if (min > load.at(i))
                min = load.at(i);
        return min;
    }

    /**
     * Get first pu in idle-
     */
    int LocalLoad::getIdlePU() {
        for (unsigned int i = 0; i < port.size(); i++)
            if (load.at(i) == 0)
                return i;
        return -1;
    }

    //End implementation

    FogLoadBalancer::FogLoadBalancer() {
        // TODO Auto-generated constructor stub

    }

    FogLoadBalancer::~FogLoadBalancer() {
    }

    void FogLoadBalancer::initialize()
        {
            nServers = getNumServers();

            localLoad = new LocalLoad(nServers);

            // FIXME: nApps is likely uninitialized!
            // what is the meaning of this code???
            //Vettore che dovrebbe tenere le porte di uscita
            //dei msg di probe. nApps si usava nel vecchio dispatcher
            //Contiene gli indici dei miei vicini a cui mandare i probe
            //Posso farci uno shuffle per selezionarne a caso un sotto gruppo di dimensioni fanout
            int probeGateSize = gateSize("loadToNeighbor");
            probeGates.resize(probeGateSize);
            for (int i = 0; i < probeGateSize; i++)
                probeGates[i] = i;

            nJobs=0; // total number of jobs
            nDroppedJobs = 0; // total number of dropped jobs
            nDroppedJobsSLA=0; // number of jobs due to SLA violations
            nLocalJobs=0; // number of jobs sent to the local PU
            nRemoteJobs=0; // number of jobs forwarded to a remote LB
            nProbes=0; // number of probe started
            nProbeQuery=0; // number of probe messages sent
            nProbeAnswers=0; // number of useful probe answers received
            queryFanOut=new cOutVector("QueryFanOut"); // for each query we record the fanOut
            answersPerQuery=new cOutVector("AnswersPerQuery"); // for each query we record how many answers we received before deciding
        }

    /**
     * Handle incoming message.
     */
    void FogLoadBalancer::handleMessage(cMessage *msg)
        {
            EV<<msg->getName()<<" arrivato al gate #"<<msg->getArrivalGate()->getIndex();
            //In case incoming msg is a job
            if (strcmp(msg->getName(), "job") == 0) {
                FogJob *job = check_and_cast<FogJob *>(msg);
                processJob(job);
            }
            //In case incoming msg is a loadUpdate
            else if (strcmp(msg->getName(), "loadUpdate") == 0) {
                LoadUpdate *loadUpdate = check_and_cast<LoadUpdate *>(msg);
                handleLoadUpdate(loadUpdate);
                dropAndDelete(msg);
            }
            //In case incoming msg is a probeQuery
            else if (strcmp(msg->getName(), "probeQuery") == 0) {
                ProbeQuery *probeQuery = check_and_cast<ProbeQuery *>(msg);
                processProbeQuery(probeQuery);
                dropAndDelete(msg);
            }
            //In case incoming msg is a probeAnswer
            else if (strcmp(msg->getName(), "probeAnswer") == 0) {
                ProbeAnswer *answer = check_and_cast<ProbeAnswer *>(msg);
                nProbeAnswersPerClass[answer->getAppId()]++;
                nProbeAnswers++; //CHECK: cosa si intende per usefull? (vedi commento di questa var in .h)
                processProbeAnswer(answer);
                dropAndDelete(msg);
            }
        }

    /**
     * Process incoming job.
     *
     * If SLA is expire and job is flag as real time -> drop it
     * Otherwise:
     *      -> If job if from another fog and multihop=false (not allowed forward), send job to an pu
     *      -> If it can be forwarded, check if i'll do it
     */
    void FogLoadBalancer::processJob(FogJob *job) {
        nJobsPerClass[job->getAppId()]++;
        nJobs++;

        if (checkSlaExpired(job)) {
            nDroppedJobsPerClass[job->getAppId()]++;
            nDroppedJobsSLAPerClass[job->getAppId()]++;
            nDroppedJobs++;
            nDroppedJobsSLA++;
            dropAndDelete(job);
        }
        else {
            //If job is from internal or it's from external and it has multihop flagged
            if (getSource(job) == INTERNAL_JOB ||
                    (getSource(job) == EXTERNAL_JOB && job->getMultiHop())) {
                if (decideProcessLocally(job)) {
                    processLocally(job);
                }
                else { // job not to process locally
                    if (decideStartProbes(job)) {
                        startProbes(job);
                    }
                    else { // job to forward to another dispatcher
                        std::vector<int> neighs = getNeighbors(getFanout());
                        int neigh = selectNeighbor(neighs);
                        forwardJob(job, (char *) "loadToNeighbor", neigh);
                        nRemoteJobsPerClass[job->getAppId()]++;
                        nRemoteJobs++;
                        //dropAndDelete(job);
                    }
                }
            }
            //If job is external and it hasn't multihop flagged
            else {
                processLocally(job);
            }
        }
    }

    /**
     * Handle probe request.
     * Send my local load.
     */
    void FogLoadBalancer::processProbeQuery(ProbeQuery *probeQuery) {
        // Check if server is free
        nProbeAnswersPerClass[probeQuery->getAppId()]++;
        nProbeAnswers++;
        int nGate = probeQuery->getArrivalGate()->getIndex();
        int load = localLoad->getLeastLoad();
        int queryId = probeQuery->getQueryId();
        int appId = probeQuery->getAppId();

        nProbeAnswersPerClass[appId]++;
        nProbeAnswers++;

        ProbeAnswer *answer = new ProbeAnswer(getAnswerName());
        answer->setLoad(load);
        answer->setQueryId(queryId);
        answer->setAppId(appId);

        send(answer, "loadToNeighbor", nGate);
    }

    /**
     * Handle probe answers
     * Fill probesQueue with answers.
     */
    void FogLoadBalancer::processProbeAnswer(ProbeAnswer *answer) {
        int fogId = answer->getArrivalGate()->getIndex();
        int load = answer->getLoad();
        int queryId = answer->getQueryId();
        int appId = answer->getAppId();

        unsigned int i;
        //When query with id==queryId isn't still active it will be == null
        if (probesQueue[queryId] != NULL) {
            for (i = 0; i < probesQueue[queryId]->neighs.size(); i++)
                if ((probesQueue[queryId]->neighs.at(i)) == fogId)
                    break;
            probesQueue[queryId]->load.at(i) = load;


            //If SLA expired, delete job and relative query
            /*
             * CHECK: Come recupero un job avendo il jobID?
             */
            FogJob *job = NULL;
            if (checkSlaExpired(job)) {
                dropAndDelete(job);
                probesQueue[queryId] = NULL;
                nDroppedJobsPerClass[appId]++;
                nDroppedJobs++;
            }
            else {
                //SLA is still ok
                if (decideForwardNow()) {
                    //forward to neigh without wait other probes
                    //so delete query
                    probesQueue[queryId] = NULL;
                    std::vector<int> neighs = getNeighbors(getFanout());
                    int neigh = selectNeighbor(neighs);
                    forwardJob(job,(char *)"loadToNeighbor" ,neigh);
                    nRemoteJobsPerClass[appId]++;
                    nRemoteJobs++;
                }
                else {
                    //Can we wait for another load?
                    //Check if last probe
                    bool lastProbe = true;
                    for (i = 0; i < probesQueue[queryId]->neighs.size(); i++)
                        if ((probesQueue[queryId]->load.at(i)) == -1) {
                            lastProbe = false;
                            break;
                        }
                    if (lastProbe) {
                        //NO, this is last ans
                        probesQueue[queryId] = NULL;
                        if (decideProcessLocally(job)) {
                            //processLocally
                            int pu = selectLocalPU();
                            forwardJob(job,(char *) "out", pu);
                            nLocalJobsPerClass[appId]++;
                            nLocalJobs++;
                        }
                        else {
                            //forward to neigh
                            std::vector<int> neighs = getNeighbors(getFanout());
                            int neigh = selectNeighbor(neighs);
                            forwardJob(job, (char *)"loadToNeighbor", neigh);
                            nRemoteJobsPerClass[appId]++;
                            nRemoteJobs++;
                        }
                    }
                }
            }
        }
    }

    bool FogLoadBalancer::decideProcessLocally(FogJob *job) {
        if (!(job->getMultiHop()))
            return true;

        if (localLoad->getLeastLoad() == 0)
            return true;
        //if (other_rules)
        //    return true;
        return false;
    }

    /**
     * To Implement in subclass.
     */
    bool FogLoadBalancer::decideStartProbes(FogJob *job) {
        return true;
    }

    /**
     * To implement in subclass
     */
    bool FogLoadBalancer::decideForwardNow() {
        return true;
    }

    /**
     * Forward job to selected pu
     */
    void FogLoadBalancer::forwardJob(FogJob *job, char *gateName, int gateIndex) {
        //updateStats
        send(job, gateName, gateIndex);
    }

    /**
     * Select neighbor which i send job.
     */
    int FogLoadBalancer::selectNeighbor(std::vector<int> neighs) {
        return 0;
    }

    void FogLoadBalancer::handleLoadUpdate(LoadUpdate *msg) {
        localLoad->updateLoad(msg->getArrivalGate()->getIndex(), msg->getBusy());
    }

    int FogLoadBalancer::getFanout() {
        return 1;
    }

    /**
     * Return job arrival gate (from external or internal)
     */
    int FogLoadBalancer::getSource(FogJob *job) {
        cGate *gate = job->getArrivalGate();
        if (strcmp(gate->getName(), "loadFromNeighbor") == 0)
            return EXTERNAL_JOB;
        return INTERNAL_JOB;
    }

    /**
     * Return how much servers there are
     */
    int FogLoadBalancer::getNumServers() {
        return gateSize("out");
    }

    /**
     * Return the list of neighs to send probe
     */
    std::vector<int> FogLoadBalancer::getNeighbors(int fanout) {
        //Randomize all neighs vector and return subvector
        //according with fanout for size
        std::random_shuffle(probeGates.begin(), probeGates.end());
        std::vector<int> neighs;
        neighs.resize(fanout);
        for (int i = 0; i < fanout; i++)
            neighs.insert(neighs.begin(), probeGates.at(i));
        return neighs;
    }

    /**
     * Select local PU
     * Implementa direttamente in coda (classe localLoad)
     */
    int FogLoadBalancer::selectLocalPU() {
        return localLoad->getLeastLoadPU();
    }

    /**
     * Return true if I have to drop job
     * So, if it is flag as realTime and if SLA expired
     */
    bool FogLoadBalancer::checkSlaExpired(FogJob *job) {
        return (job->getRealTime() && !(simTime()<=job->getSlaDeadline()));
    }

    void FogLoadBalancer::processLocally(FogJob *job) {
        int selectPU = selectLocalPU();
        if (selectPU == -1) { selectPU = 0; }
        nLocalJobsPerClass[job->getAppId()]++;
        nLocalJobs++;
        forwardJob(job, (char *) "out", selectPU);
        //dropAndDelete(job);
    }

    void FogLoadBalancer::startProbes(FogJob *job) {
        std::vector<int> neighs = getNeighbors(getFanout());
        ProbeQueue::addQuery(neighs, probesQueue, job->getId(), job);
        nProbesPerClass[job->getAppId()]++;
        nProbes++;
        for (unsigned int i = 0; i < neighs.size(); i++) {
            nProbeQueryPerClass[job->getAppId()]++;
            nProbeQuery++;
            ProbeQuery *pq = new ProbeQuery(getProbeQueryName());
            pq->setQueryId(ProbeQueue::nextQueryId-1);
            pq->setJobId(job->getId());
            pq->setAppId(job->getAppId());
            send(pq, (char *) "loadToNeighbor", neighs.at(i));
        }
    }

    void FogLoadBalancer::finish(){
        // FIXME: save statstics
        //Just a try
        recordScalar("#dropped jobs",nDroppedJobs);
        recordScalar("#process locally",nLocalJobs);
        recordScalar("#process remote",nRemoteJobs);
        recordScalar("#sended probes",nProbeQuery);
        recordScalar("#sended answers",nProbeAnswers);
    }

    const char *FogLoadBalancer::getAnswerName(){
        return "probeAnswer";
    }

    const char *FogLoadBalancer::getProbeQueryName(){
        return "probeQuery";
    }

} //namespace

