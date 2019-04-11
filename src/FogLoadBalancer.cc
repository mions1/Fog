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
#include "FogLoadBalancer.h"
#include "FogJob_m.h"
#include "FogPU.h"

namespace fog {

    Define_Module(FogLoadBalancer);
    const int INTERNAL_JOB = 0;
    const int EXTERNAL_JOB = 1;

    FogLoadBalancer::FogLoadBalancer() {
        // TODO Auto-generated constructor stub

    }

    FogLoadBalancer::~FogLoadBalancer() {
        // TODO Auto-generated destructor stub
    }

    void FogLoadBalancer::initialize()
        {
            // TODO - Generated method body
            nDroppedJobs = 0;
            nServers = getNumServers();
            nApps =  nApps = this->getParentModule()->par("nApps");

            FogPU *tmpServer = (FogPU *) this->getParentModule()->getSubmodule("server", 0);
            queueCapacity = queueCapacity = (tmpServer->capacity) * nServers;

            localLoad = new LocalLoad(nServers);

            probeGates.resize(nApps - 1);
            for (int i = 0; i < nApps - 1; i++)
                probeGates[i] = i;
        }

    void FogLoadBalancer::handleMessage(cMessage *msg)
        {
            //In case incoming msg is a job
            if (strcmp(msg->getName(), "job") == 0) {
                FogJob *job = (FogJob *) msg;
                processJob(job);
            }
        }

    /**
     * Gestisce il processamento del job arrivato
     *
     * Se lo SLA è scaduto e se il job è realtime, viene droppato
     * Altrimenti:
     *      ->Se arriva da fuori e non è possibile far forward(multihop=false) allora lo mando ad una mia pu
     *      ->Se è possibile far forward verifico se è il caso
     */
    void FogLoadBalancer::processJob(FogJob *job) {

        if (checkIfDropJob(job)) {
            dropAndDelete(job);
            nDroppedJobs++;
            //updatestats
            //slaExpired++
        }
        else {
            if (getSource(job) == EXTERNAL_JOB) {
                if (decideProcessLocally(job)) {
                    int selectPU = selectLocalPU();
                    if (selectPU == -1) selectPU = 0;
                    forwardJob(job, "out" ,selectPU);
                }
                else {
                    std::vector<int> neighs = getNeighbors(getFanout());
                    if (decideStartProbes(job)) {
                        ProbeQueue *pq = new ProbeQueue();
                        pq->addQuery(neighs);
                        //sendProbes(pq);
                    }
                    else {
                        int neigh = selectNeighbor(neighs);
                        forwardJob(job,"loadToNeighbor",neigh);
                    }
                }
            }
        }
    }

    /**
     * Ancora da implementare
     */
    void FogLoadBalancer::processProbeAnswer() {

    }

    bool FogLoadBalancer::decideProcessLocally(FogJob *job) {
        if (job->getMultiHop())
            return true;
        //if (other_rules)
        //    return true;
        return false;
    }

    /**
     * Ancora da implementare
     */
    bool FogLoadBalancer::decideStartProbes(FogJob *job) {
        return true;
    }

    /**
     * Ancora da implementare
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

    int FogLoadBalancer::selectNeighbor(std::vector<int> neighs) {
        return 0;
    }

    int FogLoadBalancer::getFanout() {
        return 3;
    }

    /**
     * Return job arrival gate (from external or internal)
     */
    int FogLoadBalancer::getSource(FogJob *job) {
        cGate *gate = job->getArrivalGate();

        if (strcmp(gate->getName(), "outerLoad") == 0)
            return EXTERNAL_JOB;
        return INTERNAL_JOB;
    }

    /**
     * Return how much servers there are
     */
    int FogLoadBalancer::getNumServers() {
        for (int i = 0;; i++) {
            if ((this->getParentModule()->getSubmodule("server", i)) == NULL)
                return i;
        }
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
     * Restituisce vero se il job è da droppare
     * Il job è da droppare se è flaggato come realTime e se è scaduto il SLA
     */
    bool FogLoadBalancer::checkIfDropJob(FogJob *job) {
        //simtime_t now = simtime();
        if (job->getRealTime() == 1 && !checkSLA(job))
            return true;
        return false;
    }

    /**
     * Restituisce vero se lo SLA è valido
     */
    bool FogLoadBalancer::checkSLA(FogJob *job) {
        return (simTime()<=job->getSlaDeadline());
    }


} //namespace

