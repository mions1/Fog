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

#ifndef __CLOUDMGN_CLOUDAPPDISPATCHERLOAD_H_
#define __CLOUDMGN_CLOUDAPPDISPATCHERLOAD_H_

#include <omnetpp.h>
#include <vector>

using namespace omnetpp;

namespace fog {

class FogDispatcherLoad : public cSimpleModule
{
private:
    int servers;
    int probeSent;
    int incID;
    int nApps;
    int queueCapacity;
    int answerIterator;
    double jobsTimedOut;
    double nJobsBlockedFromQueue;
    double nJobs;
    double nJobsSent;
    double nJobsReceived;
    double nJobsDropped;
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
    std::vector<int> probeGates;
    std::vector<int> serverBusy;
    std::vector<int> queueBusy;
    std::vector<int> ackGates;
    std::vector<int> probeAnswer;
    std::vector<int> prova;

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual ~FogDispatcherLoad();
    virtual void finish();
};

} //namespace

#endif
