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

#ifndef __FOG_LOADBALANCER_H_
#define __FOG_LOADBALANCER_H_

#include <omnetpp.h>
#include "FogJob_m.h"

using namespace omnetpp;

namespace fog {

/**
 * TODO - Generated class
 */
class FogLoadBalancer : public cSimpleModule
{

    private:
        class ProbeQueue {
            int jobs = 0;
            int queries = 0;
            public:
                int queryId;
                int jobId;
                std::vector<int> neighs;
                std::vector<bool> answer;   //Answers are mapped with neighs, so answer[0]->neigs[0]

                void addQuery(std::vector<int> neighs) {
                    queryId = queries;
                    jobId = jobs;
                    this->neighs = neighs;
                    answer.resize(neighs.size(),false);
                }
        };

        class Neighbor {
            public:
                int neigh;
                int port;
                int load;
                int rtt;
                int lastSeen;
            };

        class LocalLoad {
            public:
                std::vector<int> port;  //we consider port as pu id
                std::vector<int> load;

                LocalLoad(int nServers) {
                    port.resize(nServers, 0);
                    load.resize(nServers, 0);
                    port.reserve(nServers);
                    load.reserve(nServers);
                }

                int getLeastLoadPU() {
                    int min = load.at(0);
                    int minIndex = 0;
                    for (int i = 1; i < port.size(); i++) {
                        if (min > load.at(i)) {
                            minIndex = i;
                            min = load.at(i);
                        }
                    }
                    return minIndex;
                }

                int getIdlePU() {
                    for (int i = 0; i < port.size(); i++)
                        if (load.at(i) == 0)
                            return i;
                    return -1;
                }

        };


    public:
        FogLoadBalancer();
        virtual ~FogLoadBalancer();

    protected:
        int nServers;
        int nApps;
        int nDroppedJobs;
        int queueCapacity;

        LocalLoad *localLoad;
        std::vector<int> probeGates;

        /*
        int probeSent;
        int incID;
        int answerIterator;
        double jobsTimedOut;
        double nJobsBlockedFromQueue;
        double nJobs;
        double nJobsSent;
        double nJobsReceived;
        int fanOut;
        double blockingProbability;
        bool jobSent;
        bool serverFound;
        bool trovato;
        bool ackCheck;
        cQueue buffer;
        cMessage *probeEvent;
        cMessage *ackEvent;

        cMessage *nackEvent;
        std::vector<int> ackGates;
        std::vector<int> probeAnswer;
        std::vector<int> prova;
        */


        virtual void initialize();
        virtual void handleMessage(cMessage *msg);

        virtual void processJob(FogJob *job);   //If msg is a job, this function process it
        virtual void processProbeAnswer();      //If msg is a probeAnswer, this function process it

        virtual bool decideProcessLocally(FogJob *job); //Decide if job have to be processed locally
        virtual bool decideForwardNow();                //Decide if job have to be processed now, so without probing
        virtual bool decideStartProbes(FogJob *job);    //Decide if start probing

        virtual void forwardJob(FogJob *job, char *gateName, int gateIndex); //Forward job at the gateName port

        virtual int selectNeighbor(std::vector<int> neighbors); //Select neighbor for send job based on probing
        virtual int selectLocalPU();    //Select localPU which going to process job

        virtual bool checkIfDropJob(FogJob *job);   //Check if job is to be dropped
        bool checkSLA(FogJob *job); //Check is SLA expired

        virtual std::vector<int> getNeighbors(int fanout);  //Get neighbors to send probing
        virtual int getFanout(); //Get fanout
        virtual int getSource(FogJob *job); //Get if arrived job is from internal or external
        virtual int getNumServers();    //Get number of servers in this Fog
};

} //namespace

#endif
