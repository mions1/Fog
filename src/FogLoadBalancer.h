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
#include "ProbeAnswer_m.h"
#include "ProbeQuery_m.h"
#include "LoadUpdate_m.h"

using namespace omnetpp;

namespace fog {

class ProbeQueue {
    public:
        static int nextQueryId;
        int queryId;
        FogJob* job;
        int jobId;
        std::vector<int> neighs;
        std::vector<int> load;   //Answers are mapped with neighs, so load[0]->neigs[0]

        static void addQuery(std::vector<int> neighs, std::map<int, ProbeQueue*> probesQueue, int jobId, FogJob* job);
};

// FIXME: questa classe non è mai usata.
class Neighbor {
    public:
        int neigh; // FIXME: neigh, port: probabilmente uno dei due è superfluo.
        int port; // FIXME: Verificare se il tipo è unsigned.
        int load; // FIXME: il tipo dovrebbe essere float
        int rtt; // FIXME: il tipo dovrebbe essere simtime_t
        int lastSeen; // FIXME: il tipo dovrebbe essere simtime_t
    };

class LocalLoad {
    public:
        std::vector<int> port;  //we consider port as pu id. Ma serve questo vettore??
        std::vector<int> load;

        LocalLoad(int nServers);

        void updateLoad(int pu, int newLoad);
        int getLeastLoadPU();
        int getLeastLoad();
        int getIdlePU();
};

/**
 * TODO - Generated class
 */
class FogLoadBalancer : public cSimpleModule
{

    public:
        FogLoadBalancer();
        virtual ~FogLoadBalancer();

    protected:
        int nServers;
        int nApps;
        int queueCapacity;

        LocalLoad *localLoad;
        std::vector<int> probeGates;
        std::map<int, ProbeQueue *> probesQueue;

        // FIXME: Should be per-class
        std::map<int, int> nJobsPerClass;
        std::map<int, int> nLocalJobsPerClass;
        std::map<int, int> nRemoteJobsPerClass;
        std::map<int, int> nDroppedJobsPerClass;
        std::map<int, int> nDroppedJobsSLAPerClass;
        std::map<int, int> nProbesPerClass;
        std::map<int, int> nProbeQueryPerClass;
        std::map<int, int> nProbeAnswersPerClass;

        int nJobs; // total number of jobs v
        int nDroppedJobs;
        int nDroppedJobsSLA; // number of jobs due to SLA violations
        int nLocalJobs; // number of jobs sent to the local PU v
        int nRemoteJobs; // number of jobs forwarded to a remote LB v
        int nProbes; // number of probe started v
        int nProbeQuery; // number of probe messages sent v
        int nProbeAnswers; // number of useful probe answers received v
        cOutVector *queryFanOut; // for each query we record the fanOut
        cOutVector *answersPerQuery; // for each query we record how many answers we received before deciding
        //cOutVector *localLoadAtQuery; // for each query we record the lowest local load
        //cOutVector *remoteLoadAtQuery; // for each query we record the remote load
        /*
        double jobsTimedOut;
        double nJobsBlockedFromQueue;
        double blockingProbability;
        */


        virtual void initialize();
        virtual void handleMessage(cMessage *msg);

        virtual void processJob(FogJob *job);   //If msg is a job, this function process it
        virtual void processProbeAnswer(ProbeAnswer *answer);      //If msg is a probeAnswer, this function process it
        virtual void processProbeQuery(ProbeQuery *probeQuery); //If msg is a probe request

        virtual bool decideProcessLocally(FogJob *job); //Decide if job have to be processed locally
        virtual bool decideForwardNow();                //Decide if job have to be processed now, so without probing
        virtual bool decideStartProbes(FogJob *job);    //Decide if start probing

        virtual void forwardJob(FogJob *job, char *gateName, int gateIndex); //Forward job at the gateName port

        virtual int selectNeighbor(std::vector<int> neighbors); //Select neighbor for send job based on probing
        virtual int selectLocalPU();    //Select localPU which going to process job

        void handleProbeQuery(cMessage *msg);

        void handleLoadUpdate(LoadUpdate *loadUpdate);
        virtual void finish();

        virtual bool checkSlaExpired(FogJob *job);   //Check if job is to be dropped

        void startProbes(FogJob *job);
        void processLocally(FogJob *job);

        virtual std::vector<int> getNeighbors(int fanout);  //Get neighbors to send probing
        virtual int getFanout(); //Get fanout
        virtual int getSource(FogJob *job); //Get if arrived job is from internal or external
        virtual int getNumServers();    //Get number of servers in this Fog

        static const char *getAnswerName();
        static const char *getProbeQueryName();
};

} //namespace

#endif
