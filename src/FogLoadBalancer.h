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
#include <string>
#include "FogJob_m.h"
#include "ProbeAnswer_m.h"
#include "ProbeQuery_m.h"
#include "LoadUpdate_m.h"

using namespace omnetpp;

namespace fog {

/**
 * Class to manage probe queue
 */
class ProbeQueue {
    public:
        static int nextQueryId; // auto increment id for enqueued query

        int queryId;
        FogJob* job;    // enqueued job
        int jobId;      // job id of enqueued job
        std::vector<int> neighs;    // list of neighs to which the probe has been sent
        std::vector<int> load;   //Neighs' answers, these are mapped with neighs, so load[0]->neigs[0]

        //Add a new query to queue
        static void addQuery(std::vector<int> neighs, std::map<int, ProbeQueue*> probesQueue, int jobId, FogJob* job);
        //Giving a jobid, return its probe queue
        static ProbeQueue* getQueueFromJobId(int jobId, std::map<int, ProbeQueue *> probesQueue);
        //Get least load PU
        int getLeastLoadPU();
        //Get least load
        int getLeastLoad();
};

/**
 * Class to manage localLoad (load of local PU)
 */
class LocalLoad {
    public:

        int lastPu;             // last PU to which the job was sent
        std::vector<int> load;  // load for each pu

        LocalLoad(int nServers);

        //Update load of a PU
        void updateLoad(int pu, int newLoad);
        //Get least load PU
        int getLeastLoadPU(bool readOnly);
        //Get least load
        int getLeastLoad();
        //Get a idle PU
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
        int nServers;       //PU number
        int nApps;          // Appid number
        int queueCapacity;  // PU's queue capacity

        LocalLoad *localLoad;
        std::vector<int> probeGates;    // gates for neighs
        std::map<int, ProbeQueue *> probesQueue;

        //Statics
            // per class
        std::map<int, int> nJobsPerClass;       // total number of jobs
        std::map<int, int> nLocalJobsPerClass;  // total number of jobs sent to local PU
        std::map<int, int> nRemoteJobsPerClass; // total number of jobs forwarded to another LB
        std::map<int, int> nDroppedJobsSLAPerClass; // number of jobs dropped due to SLA violations
        std::map<int, int> nProbesPerClass;     // number of probes started
        std::map<int, int> nProbeQueryPerClass; // number of probes messages sent
        std::map<int, int> nProbeAnswersPerClass;   // number of answers received

        int nJobs; // total number of jobs
        int nInternalJobs; // total number of jobs arrived from source
        int nExternalJobs; // total number of jobs arrived from another LB
        int nDroppedJobsSLA; // number of jobs due to SLA violations
        int nLocalJobs; // number of jobs sent to the local PU
        int nRemoteJobs; // number of jobs forwarded to a remote LB
        int nProbes; // number of probe started
        int nProbeQuery; // number of probe messages sent
        int nProbeAnswers; // number of useful probe answers received

        virtual void initialize();
        virtual void handleMessage(cMessage *msg);

        virtual void processJob(FogJob *job);   // if incoming msg is a job, this function process it
        virtual void processProbeAnswer(ProbeAnswer *answer);      // if incoming msg is a probeAnswer, this function process it
        virtual void processProbeQuery(ProbeQuery *probeQuery); // if incoming msg is a probe request

        void handleLoadUpdate(LoadUpdate *loadUpdate);  // when arrive a local load update

        virtual bool decideProcessLocally(FogJob *job); // decide if job have to be processed locally
        virtual bool decideForwardNow(FogJob* job, int queryId);    // decide if job have to be forward now, so without wait other probes
        virtual bool decideStartProbes(FogJob *job);    // decide if start probing

        void startProbes(FogJob *job);
        void processLocally(FogJob *job);
        virtual void forwardJob(FogJob *job, char *gateName, int gateIndex); // forward job at the gateName port

        virtual int selectNeighbor(std::vector<int> neighbors); // select neighbor to send job based on probing
        virtual int selectLocalPU();    // select localPU which going to process job

        virtual bool checkSlaExpired(FogJob *job);   // check if job is to be dropped due SLA violation
        virtual std::vector<int> getNeighbors(int fanout);  // get list of neighbors to send probing
        virtual int getFanout(); // get fanout
        int getNumServers();    // get number of servers in this Fog node
        int getSource(FogJob *job); // says if arrived job is from internal (source) or from external (other LB)

        void dumpStat(std::map<int, int> *map, std::string name);
        virtual void finish();

        static const char *getAnswerName();
        static const char *getProbeQueryName();

};

} //namespace

#endif
