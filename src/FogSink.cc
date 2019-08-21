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

void FogSink::initMaps(int appId){
    if (responseTimeVector.find(appId) == responseTimeVector.end()){
        std::string name="responseTime_" + std::to_string(appId);
        responseTimeVector[appId]=new cOutVector(name.c_str());
    }
    if (delayTimeVector.find(appId) == delayTimeVector.end()){
        std::string name="delayTime_" + std::to_string(appId);
        delayTimeVector[appId]=new cOutVector(name.c_str());
    }
    if (queuingTimeVector.find(appId) == queuingTimeVector.end()){
        std::string name="queuingTime_" + std::to_string(appId);
        queuingTimeVector[appId]=new cOutVector(name.c_str());
    }
    if (serviceTimeVector.find(appId) == serviceTimeVector.end()){
        std::string name="serviceTime_" + std::to_string(appId);
        serviceTimeVector[appId]=new cOutVector(name.c_str());
    }
    if (balancerTimeVector.find(appId) == balancerTimeVector.end()){
        std::string name="balancerTime_" + std::to_string(appId);
        balancerTimeVector[appId]=new cOutVector(name.c_str());
    }
    if (balancerCountVector.find(appId) == balancerCountVector.end()){
        std::string name="balancerCount_" + std::to_string(appId);
        balancerCountVector[appId]=new cOutVector(name.c_str());
    }
    if (responseTimeStat.find(appId) == responseTimeStat.end()){
        responseTimeStat[appId]=new cPSquare();
    }
    if (delayTimeStat.find(appId) == delayTimeStat.end()){
        delayTimeStat[appId]=new cPSquare();
    }
    if (queuingTimeStat.find(appId) == queuingTimeStat.end()){
        queuingTimeStat[appId]=new cPSquare();
    }
    if (serviceTimeStat.find(appId) == serviceTimeStat.end()){
        serviceTimeStat[appId]=new cPSquare();
    }
    if (balancerTimeStat.find(appId) == balancerTimeStat.end()){
        balancerTimeStat[appId]=new cPSquare();
    }
    if (balancerCountStat.find(appId) == balancerCountStat.end()){
        balancerCountStat[appId]=new cPSquare();
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
    /*if (responseTimeVector.find(appId) == responseTimeVector.end()){
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
    }*/
    // FIXME: linking error in these functions
    //initVector(& responseTimeVector, appId, "responseTime");
    //initVector(& delayTimeVector, appId, "delayTime");
    //initVector(& queuingTimeVector, appId, "queuingTime");
    //initVector(& serviceTimeVector, appId, "serviceTime");
    //initVector(& balancerTimeVector, appId, "balancerTime");
    initMaps(appId);
    responseTimeVector[appId]->recordWithTimestamp(now, t);
    delayTimeVector[appId]->recordWithTimestamp(now, job->getDelayTime());
    queuingTimeVector[appId]->recordWithTimestamp(now, job->getQueuingTime());
    serviceTimeVector[appId]->recordWithTimestamp(now, job->getServiceTime());
    balancerTimeVector[appId]->recordWithTimestamp(now, job->getServiceTime());
    balancerCountVector[appId]->recordWithTimestamp(now, job->getBalancerCount());
    responseTimeStat[appId]->collect(t);
    delayTimeStat[appId]->collect(job->getDelayTime());
    queuingTimeStat[appId]->collect(job->getQueuingTime());
    serviceTimeStat[appId]->collect(job->getServiceTime());
    balancerTimeStat[appId]->collect(job->getBalancerTime());
    balancerCountStat[appId]->collect(job->getBalancerCount());
    delete msg;
}

double FogSink::getPercentile(cAbstractHistogram *hist, float val){
    return hist->getBinEdge((int) val*hist->getNumBins());
}

void FogSink::dumpStat(std::map<int,cPSquare *> *map, std::string name){
    for(std::map<int,cPSquare *>::iterator i = map->begin(); i != map->end(); ++i){
        int appId=i->first;
        cPSquare *stats=i->second;
        std::string avgname = "avg_" + name + "_" + std::to_string(appId);
        std::string medname = "p90_" + name + "_" + std::to_string(appId);
        std::string p90name = "med_" + name + "_" + std::to_string(appId);
        recordScalar(avgname.c_str(), stats->getMean());
        recordScalar(medname.c_str(), getPercentile(stats, 0.5));
        recordScalar(p90name.c_str(), getPercentile(stats, 0.9));
    }
}

void FogSink::cleanStat(std::map<int,cPSquare *> *map){
    for(std::map<int,cPSquare *>::iterator i = map->begin(); i != map->end(); ++i){
        delete i->second;
    }
}

void FogSink::cleanVector(std::map<int,cOutVector *> *map){
    for(std::map<int,cOutVector *>::iterator i = map->begin(); i != map->end(); ++i){
        delete i->second;
    }
}

void FogSink::finish()
{
    dumpStat(& responseTimeStat, "responseTime");
    dumpStat(& delayTimeStat, "delayTime");
    dumpStat(& queuingTimeStat, "queuingTime");
    dumpStat(& serviceTimeStat, "serviceTime");
    dumpStat(& balancerTimeStat, "balancerTime");
    dumpStat(& balancerCountStat, "balancerCount");
    cleanStat(& responseTimeStat);
    cleanStat(& delayTimeStat);
    cleanStat(& queuingTimeStat);
    cleanStat(& serviceTimeStat);
    cleanStat(& balancerTimeStat);
    cleanStat(& balancerCountStat);
    cleanVector(& responseTimeVector);
    cleanVector(& delayTimeVector);
    cleanVector(& queuingTimeVector);
    cleanVector(& serviceTimeVector);
    cleanVector(& balancerTimeVector);
    cleanVector(& balancerCountVector);
}

}; // namespace
