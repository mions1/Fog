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
#include "CloudSink.h"
#include "FogJob_m.h"


namespace fog {

Define_Module(CloudSink);

CloudSink::~CloudSink()
{
    int i;
    for(i=0; i<nApps; i++){
        // de-allocate data structures
        delete responseTimeVector[i];
        delete delayTimeVector[i];
        delete queuingTimeVector[i];
        delete serviceTimeVector[i];
        delete responseTimeStat[i];
        delete delayTimeStat[i];
        delete queuingTimeStat[i];
        delete serviceTimeStat[i];
    }
}

void CloudSink::initialize()
{
    int i;
    nApps=par("maxApps");
    responseTimeVector.resize(nApps);
    delayTimeVector.resize(nApps);
    queuingTimeVector.resize(nApps);
    serviceTimeVector.resize(nApps);
    responseTimeStat.resize(nApps);
    delayTimeStat.resize(nApps);
    queuingTimeStat.resize(nApps);
    serviceTimeStat.resize(nApps);
    for(i=0; i<nApps; i++){
        std::ostringstream rtname; //response time vector name
        std::ostringstream dtname; //delay time vector name
        std::ostringstream qtname; //queuing time vector name
        std::ostringstream stname; //service time vector name
        rtname << "responseTime_" << i << std::ends;
        dtname << "delayTime_" << i << std::ends;
        qtname << "queuingTime_" << i << std::ends;
        stname << "serviceTime_" << i << std::ends;
        // cOutVector
        responseTimeVector[i] = new cOutVector(rtname.str().c_str());
        delayTimeVector[i] = new cOutVector(dtname.str().c_str());
        queuingTimeVector[i] = new cOutVector(qtname.str().c_str());
        serviceTimeVector[i] = new cOutVector(stname.str().c_str());
        // cPSquare
        responseTimeStat[i] = new cPSquare(rtname.str().c_str());
        delayTimeStat[i] = new cPSquare(dtname.str().c_str());
        queuingTimeStat[i] = new cPSquare(qtname.str().c_str());
        serviceTimeStat[i] = new cPSquare(stname.str().c_str());
    }
}

void CloudSink::handleMessage(cMessage *msg)
{
    // cast message to CloudAppJob
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
    responseTimeVector[appId]->recordWithTimestamp(now, t);
    delayTimeVector[appId]->recordWithTimestamp(now, job->getDelayTime());
    queuingTimeVector[appId]->recordWithTimestamp(now, job->getQueuingTime());
    serviceTimeVector[appId]->recordWithTimestamp(now, job->getServiceTime());
    responseTimeStat[appId]->collect(t);
    delayTimeStat[appId]->collect(job->getDelayTime());
    queuingTimeStat[appId]->collect(job->getQueuingTime());
    serviceTimeStat[appId]->collect(job->getServiceTime());

    delete msg;
}

double CloudSink::get90percentile(cAbstractHistogram *hist){
    return hist->getBinEdge((int) 0.9*hist->getNumBins());
}

void CloudSink::finish()
{
    int i;
    for(i=0; i<nApps; i++){
        std::ostringstream avgrtname; //response time vector name
        std::ostringstream avgdtname; //delay time vector name
        std::ostringstream avgqtname; //queuing time vector name
        std::ostringstream avgstname; //service time vector name
        std::ostringstream p90rtname; //response time vector name
        std::ostringstream p90dtname; //delay time vector name
        std::ostringstream p90qtname; //queuing time vector name
        std::ostringstream p90stname; //service time vector name
        avgrtname << "avg_responseTime_" << i << std::ends;
        avgdtname << "avg_delayTime_" << i << std::ends;
        avgqtname << "avg_queuingTime_" << i << std::ends;
        avgstname << "avg_serviceTime_" << i << std::ends;
        p90rtname << "p90_responseTime_" << i << std::ends;
        p90dtname << "p90_delayTime_" << i << std::ends;
        p90qtname << "p90_queuingTime_" << i << std::ends;
        p90stname << "p90_serviceTime_" << i << std::ends;
        recordScalar(avgrtname.str().c_str(), responseTimeStat[i]->getMean());
        recordScalar(avgdtname.str().c_str(), delayTimeStat[i]->getMean());
        recordScalar(avgqtname.str().c_str(), queuingTimeStat[i]->getMean());
        recordScalar(avgstname.str().c_str(), serviceTimeStat[i]->getMean());
        recordScalar(p90rtname.str().c_str(), get90percentile(responseTimeStat[i]));
        recordScalar(p90dtname.str().c_str(), get90percentile(delayTimeStat[i]));
        recordScalar(p90qtname.str().c_str(), get90percentile(queuingTimeStat[i]));
        recordScalar(p90stname.str().c_str(), get90percentile(serviceTimeStat[i]));
        /* DEPRECATED CODE
        recordScalar(p90rtname.str().c_str(), responseTimeStat[i]->getBasepoint(9));
        recordScalar(p90dtname.str().c_str(), delayTimeStat[i]->getBasepoint(9));
        recordScalar(p90qtname.str().c_str(), queuingTimeStat[i]->getBasepoint(9));
        recordScalar(p90stname.str().c_str(), serviceTimeStat[i]->getBasepoint(9));
         */
    }
}

}; // namespace
