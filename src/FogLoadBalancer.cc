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
#include <string>
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

    int ProbeQueue::nextQueryId = 0; // needed for using this static member

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

    /**
     * Giving job id, return its probeQueue
     */
    ProbeQueue* ProbeQueue::getQueueFromJobId(int jobId, std::map<int, ProbeQueue*> pq) {
        std::map<int, ProbeQueue*>::iterator it;
        for ( it = pq.begin(); it != pq.end(); it++ ) {
            if (it->second->jobId == jobId)
                return it->second;
        }
        return NULL;
    }

    /**
     * Get least load PU from queue
     */
    int ProbeQueue::getLeastLoadPU() {
        int min = load.at(0);
        int minIndex = 0;
        for (unsigned int i = 1; i < neighs.size(); i++) {
            if (min > load.at(i) && load.at(i) != -1) {
                minIndex = i;
                min = load.at(i);
            }
        }
        return minIndex;
    }

    /**
     * Get least load from queue
     */
    int ProbeQueue::getLeastLoad() {
        int min = load.at(0);
        for (unsigned int i = 1; i < neighs.size(); i++) {
            if (min > load.at(i) && load.at(i) != -1) {
                min = load.at(i);
            }
        }
        return min;
    }

    LocalLoad::LocalLoad(int nServers) {
        load.resize(nServers, 0);
        load.reserve(nServers);
        lastPu = 0;
    }

    /**
     * Set new load of the pu
     */
    void LocalLoad::updateLoad(int pu, int newLoad) {
        load[pu] = newLoad;
    }

    /**
     * Get least load PU
     * @param readOnly: if True, lastPU is not increased
     */
    int LocalLoad::getLeastLoadPU(bool readOnly) {
        int min = load[lastPu];
        int minIndex = lastPu;
        for (unsigned int i = lastPu+1; i < load.size()+lastPu; i++) {
            if (min > load.at(i%load.size())) {
                minIndex = i%load.size();
                min = load.at(i%load.size());
            }
        }
        if (!readOnly)
            lastPu = (minIndex+1)%load.size();
        return minIndex;
    }

    /**
     * Get least load value.
     */
    int LocalLoad::getLeastLoad() {
        int min = load.at(0);
        for (unsigned int i = 1; i < load.size(); i++)
            if (min > load.at(i))
                min = load.at(i);
        return min;
    }

    /**
     * Get first pu in idle
     */
    int LocalLoad::getIdlePU() {
        for (unsigned int i = 0; i < load.size(); i++)
            if (load.at(i) == 0)
                return i;
        return -1;
    }

    //------------------------------------------------------------

    FogLoadBalancer::FogLoadBalancer() {
        // TODO Auto-generated constructor stub

    }

    FogLoadBalancer::~FogLoadBalancer() {
    }

    /**
     * Initialize variables
     */
    void FogLoadBalancer::initialize()
        {
            nServers = getNumServers();
            localLoad = new LocalLoad(nServers);

            // Get neighs number and fill probeGates
            int probeGateSize = gateSize("loadToNeighbor");
            probeGates.resize(probeGateSize);
            for (int i = 0; i < probeGateSize; i++)
                probeGates[i] = i;

            nJobs=0;
            nInternalJobs=0;
            nExternalJobs=0;
            nDroppedJobsSLA=0;
            nLocalJobs=0;
            nRemoteJobs=0;
            nProbes=0;
            nProbeQuery=0;
            nProbeAnswers=0;
        }

    /**
     * Handle incoming message.
     * They can be:
     *  > job
     *  > loadUpdate
     *  > probeQuery
     *  > probeAnswer
     */
    void FogLoadBalancer::handleMessage(cMessage *msg)
        {
            //In case incoming msg is a job
            if (strcmp(msg->getName(), "job") == 0) {
                FogJob *job = check_and_cast<FogJob *>(msg);
                job->setBalancerCount(job->getBalancerCount()+1);
                processJob(job);
            }
            //In case incoming msg is a loadUpdate
            else if (strcmp(msg->getName(), "loadUpdate") == 0) {
                LoadUpdate *loadUpdate = check_and_cast<LoadUpdate *>(msg);
                handleLoadUpdate(loadUpdate);
                delete msg;
            }
            //In case incoming msg is a probeQuery
            else if (strcmp(msg->getName(), "probeQuery") == 0) {
                ProbeQuery *probeQuery = check_and_cast<ProbeQuery *>(msg);
                processProbeQuery(probeQuery);
                delete msg;
            }
            //In case incoming msg is a probeAnswer
            else if (strcmp(msg->getName(), "probeAnswer") == 0) {
                ProbeAnswer *answer = check_and_cast<ProbeAnswer *>(msg);
                nProbeAnswersPerClass[answer->getAppId()]++;
                nProbeAnswers++;
                processProbeAnswer(answer);
                delete msg;
            }
        }

    /**
     * Process incoming job.
     *
     * If SLA is expired and job is flagged as real time -> drop it
     * Otherwise:
     *      -> If job is from another fog and multihop=false (forward not allowed), send job to a local pu
     *      -> If it can be forwarded, check if do it (or probing)
     */
    void FogLoadBalancer::processJob(FogJob *job) {
        nJobsPerClass[job->getAppId()]++;
        nJobs++;
        
        if (getSource(job) == INTERNAL_JOB)
            nInternalJobs++;
        else if (getSource(job) == EXTERNAL_JOB)
            nExternalJobs++;

        if (checkSlaExpired(job)) {
            nDroppedJobsSLAPerClass[job->getAppId()]++;
            nDroppedJobsSLA++;
            dropAndDelete(job);
        }
        else {
            //If job is from internal, initialize timeStamp to compute balancerTime
            if (getSource(job) == INTERNAL_JOB)
                job->setTimestamp();
            //If job is from internal or it's from external and it has multihop flagged
            if (getSource(job) == INTERNAL_JOB ||
                    (getSource(job) == EXTERNAL_JOB && job->getMultiHop())) {
                if (decideProcessLocally(job)) {
                    processLocally(job);
                }
                else { // not to processing locally
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
     * Send my least local load.
     */
    void FogLoadBalancer::processProbeQuery(ProbeQuery *probeQuery) {
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
        //When query with id==queryId is no longer active it will be == null
        if (probesQueue[queryId] != NULL) {
            for (i = 0; i < probesQueue[queryId]->neighs.size(); i++)
                if ((probesQueue[queryId]->neighs.at(i)) == fogId)
                    break;
            probesQueue[queryId]->load.at(i) = load;


            //If SLA is expired, delete job and its query
            FogJob *job = probesQueue[queryId]->job;
            if (checkSlaExpired(job)) {
                dropAndDelete(job);
                probesQueue[queryId] = NULL;
            }
            else {  // SLA is ok
                if (decideForwardNow(job, queryId)) {
                    //forward to neigh without wait other probes
                    //so delete query
                    probesQueue[queryId] = NULL;
                    int neigh = probesQueue[queryId]->getLeastLoadPU();
                    forwardJob(job,(char *)"loadToNeighbor" ,neigh);
                    nRemoteJobsPerClass[appId]++;
                    nRemoteJobs++;
                }
                else { //can we wait for another load?
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
                            //forward to leastload neigh
                            int leastLoadNeigh = probesQueue[queryId]->getLeastLoadPU();
                            int neigh = leastLoadNeigh > -1 ? leastLoadNeigh : 0;
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
     * To redefine
     */
    bool FogLoadBalancer::decideStartProbes(FogJob *job) {
        return true;
    }

    /**
     * To redefine
     */
    bool FogLoadBalancer::decideForwardNow(FogJob *job, int queryId) {
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
     * Select the neighbor to send the job to
     */
    int FogLoadBalancer::selectNeighbor(std::vector<int> neighs) {
        return 0;
    }

    /**
     * Handle local load update.
     */
    void FogLoadBalancer::handleLoadUpdate(LoadUpdate *msg) {
        localLoad->updateLoad(msg->getArrivalGate()->getIndex(), msg->getBusy());
    }

    int FogLoadBalancer::getFanout() {
        return 1;
    }

    /**
     * Return job's arrival gate (from external or internal)
     */
    int FogLoadBalancer::getSource(FogJob *job) {
        cGate *gate = job->getArrivalGate();
        if (strcmp(gate->getName(), "loadFromNeighbor") == 0) {
            return EXTERNAL_JOB;
        }
        return INTERNAL_JOB;
    }

    /**
     * Return how much servers there are
     */
    int FogLoadBalancer::getNumServers() {
        return gateSize("out");
    }

    /**
     * Return a list of neighs to send probe to
     */
    std::vector<int> FogLoadBalancer::getNeighbors(int fanout) {
        //Randomize the vector containing all neighs and return subvector
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
     */
    int FogLoadBalancer::selectLocalPU() {
        return localLoad->getLeastLoadPU(false);
    }

    /**
     * Return true if I have to drop job
     * So, if it is flag as realTime and if SLA is expired
     */
    bool FogLoadBalancer::checkSlaExpired(FogJob *job) {
        return (job->getRealTime() && !(simTime()<=job->getSlaDeadline()));
    }

    /**
     * Process job locally.
     * Select a local pu (with selectLocalPU() function) and send job to it
     */
    void FogLoadBalancer::processLocally(FogJob *job) {
        int selectPU = selectLocalPU();
        if (selectPU == -1) { selectPU = 0; }
        nLocalJobsPerClass[job->getAppId()]++;
        nLocalJobs++;
        simtime_t now = simTime();
        simtime_t ts = job->getTimestamp();
        job->setBalancerTime(now-ts);
        forwardJob(job, (char *) "out", selectPU);
        //dropAndDelete(job);
    }

    /**
     * Start probes for this job.
     * Fill up probeQueue with a new query
     */
    void FogLoadBalancer::startProbes(FogJob *job) {
        int fanout = getFanout();
        std::vector<int> neighs = getNeighbors(fanout);
        ProbeQueue::addQuery(neighs, probesQueue, job->getId(), job);
        nProbesPerClass[job->getAppId()]++;
        nProbes++;
        //Send to neighs a probe request
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

    void FogLoadBalancer::dumpStat(std::map<int, int> *map, std::string name){
        for(std::map<int, int>::iterator i = map->begin(); i != map->end(); ++i){
            int appId=i->first;
            int value=i->second;
            //std::string recname = name + "_appId: " + std::to_string(appId);
            std::string recname = name + "_" + std::to_string(appId);
            recordScalar(recname.c_str(), value);
        }
    }

    void FogLoadBalancer::finish(){
        /*recordScalar("#process locally",nLocalJobs);
        recordScalar("#process remote",nRemoteJobs);
        recordScalar("#sent probes",nProbeQuery);
        recordScalar("#sent answers",nProbeAnswers);

        dumpStat(&nJobsPerClass, "#jobs per class");
        dumpStat(&nRemoteJobsPerClass, "#process remote per class");
        dumpStat(&nLocalJobsPerClass, "#process locally per class");
        dumpStat(&nDroppedJobsSLAPerClass, "#dropped jobs sla per class");
        dumpStat(&nProbesPerClass, "#starting probes per class");
        dumpStat(&nProbeQueryPerClass, "#sent probes query per class");
        dumpStat(&nProbeAnswersPerClass, "#sent answers per class");*/

        recordScalar("localJobs",nLocalJobs);
        recordScalar("internalJobs",nInternalJobs);
        recordScalar("externalJobs",nExternalJobs);
        recordScalar("remoteJobs",nRemoteJobs);
        recordScalar("probeQueries",nProbeQuery);
        recordScalar("probeAnswers",nProbeAnswers);
        recordScalar("droppedJobsSLA", nDroppedJobsSLA);

        dumpStat(&nJobsPerClass, "jobs_class");
        dumpStat(&nRemoteJobsPerClass, "remoteJobs_class");
        dumpStat(&nLocalJobsPerClass, "localJobs_class");
        dumpStat(&nDroppedJobsSLAPerClass, "droppedJobsSLA_class");
        dumpStat(&nProbesPerClass, "probes_class");
        dumpStat(&nProbeQueryPerClass, "probeQueries_class");
        dumpStat(&nProbeAnswersPerClass, "probeAnswers_class");
    }

    const char *FogLoadBalancer::getAnswerName(){
        return "probeAnswer";
    }

    const char *FogLoadBalancer::getProbeQueryName(){
        return "probeQuery";
    }

} //namespace

