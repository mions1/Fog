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
#include <string>
#include "FogJob_m.h"
#include "FogSink.h"


namespace fog {

Define_Module(FogSink);

FogSink::~FogSink()
{
}

void FogSink::initialize()
{
}

void initVector(std::map<int, cOutVector> *map, int appId, std::string name){
    if (map->find(appId) == map->end()){
        std::string n = name + "_" + std::to_string(appId);
        map->emplace(appId, n.c_str());
    }
}

void FogSink::handleMessage(cMessage *msg)
{
    // cast message to FogJob
    int appId;
    simtime_t t, now;
    FogJob *job=check_and_cast<FogJob*>(msg);
    appId=job->getAppId();
    now=simTime();
    t=now - job->getStartTime();
    /*EV << "JOB appId=" << appId <<
          ", now="<< now <<
          ", sent="<< job->getStartTime() <<
          ", rt="<< t <<
          ", del="<< job->getDelayTime() <<
          ", que="<< job->getQueuingTime() <<
          ", ser="<< job->getServiceTime();*/
    // update statistics
    // We should make sure that the vector name has been initialized correctly
    if (responseTimeVector.find(appId) == responseTimeVector.end()){
        std::string name="responseTime_" + std::to_string(appId);
        responseTimeVector.emplace(appId, name.c_str());
    }
    if (delayTimeVector.find(appId) == delayTimeVector.end()){
        std::string name="delayTime_" + std::to_string(appId);
        delayTimeVector.emplace(appId, name.c_str());
    }
    if (queuingTimeVector.find(appId) == queuingTimeVector.end()){
        std::string name="queuingTime_" + std::to_string(appId);
        queuingTimeVector.emplace(appId, name.c_str());
    }
    if (serviceTimeVector.find(appId) == serviceTimeVector.end()){
        std::string name="serviceTime_" + std::to_string(appId);
        serviceTimeVector.emplace(appId, name.c_str());
    }
    if (balancerTimeVector.find(appId) == balancerTimeVector.end()){
        std::string name="balancerTime_" + std::to_string(appId);
        balancerTimeVector.emplace(appId, name.c_str());
    }
    // FIXME: linking error in these functions
    //initVector(& responseTimeVector, appId, "responseTime");
    //initVector(& delayTimeVector, appId, "delayTime");
    //initVector(& queuingTimeVector, appId, "queuingTime");
    //initVector(& serviceTimeVector, appId, "serviceTime");
    //initVector(& balancerTimeVector, appId, "balancerTime");
    responseTimeVector[appId].recordWithTimestamp(now, t);
    delayTimeVector[appId].recordWithTimestamp(now, job->getDelayTime());
    queuingTimeVector[appId].recordWithTimestamp(now, job->getQueuingTime());
    serviceTimeVector[appId].recordWithTimestamp(now, job->getServiceTime());
    balancerTimeVector[appId].recordWithTimestamp(now, job->getServiceTime());
    responseTimeStat[appId].collect(t);
    delayTimeStat[appId].collect(job->getDelayTime());
    queuingTimeStat[appId].collect(job->getQueuingTime());
    serviceTimeStat[appId].collect(job->getServiceTime());
    balancerTimeStat[appId].collect(job->getServiceTime());

    delete msg;
}

double FogSink::get90percentile(cAbstractHistogram *hist){
    return hist->getBinEdge((int) 0.9*hist->getNumBins());
}

void FogSink::dumpStat(std::map<int,cPSquare> *map, std::string name){
    for(std::map<int,cPSquare>::iterator i = map->begin(); i != map->end(); ++i){
        int appId=i->first;
        cPSquare *stats=&(i->second);
        std::string avgname = "avg_" + name + "_" + std::to_string(appId);
        std::string p90name = "p90_" + name + "_" + std::to_string(appId);
        recordScalar(avgname.c_str(), stats->getMean());
        recordScalar(p90name.c_str(), get90percentile(stats));
    }
}

void FogSink::finish()
{
    dumpStat(& responseTimeStat, "responseTime");
    dumpStat(& delayTimeStat, "delayTime");
    dumpStat(& queuingTimeStat, "queuingTime");
    dumpStat(& serviceTimeStat, "serviceTime");
    dumpStat(& balancerTimeStat, "balancerTime");

}

}; // namespace
