//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include "headers/FogPUTS.h"
#include "FogJob_m.h"
#include "CloudCongestionUpdate_m.h"

using namespace omnetpp;

// transitions of server state
#define BUSY2IDLE 1
#define IDLE2BUSY 2
#define BUSY2BUSY 3
#define IDLE2IDLE 4
#define ANY2IDLE 5
#define ANY2BUSY 6

// should be the same name of .ned file
#define CONGESTIONGATENAME "congestionControl"

#define REMAININGTIME "remaining-time"


namespace fog {

Define_Module(FogPUTS);

FogPUTS::FogPUTS() {
    jobServiced = NULL;
    endServiceMsg = NULL;
    contextSwitchMsg=NULL;
    timeoutMsg=NULL;
}

FogPUTS::~FogPUTS() {
    delete jobServiced;
    cancelAndDelete(endServiceMsg);
    cancelAndDelete(contextSwitchMsg);
    cancelAndDelete(timeoutMsg);

}

void FogPUTS::initialize() {
    //droppedSignal = registerSignal("dropped");
    //queueingTimeSignal = registerSignal("queueingTime");
    //queueLengthSignal = registerSignal("queueLength");
    //emit(queueLengthSignal, 0);
    //busySignal = registerSignal("busy");
    //emit(busySignal, 0);
    busy=false;
    congested=false;
    congestionMultiplier=1.0;
    totalBusyTime=0;
    startBusyTime=0;
    totalIdleTime=0;
    startIdleTime=simTime();
    startCongestionTime=0;
    startNCongestionTime=simTime();
    totalCongestionTime=0;
    totalNCongestionTime=0;
    totalJobs=0;
    droppedJobsQueue=0;
    droppedJobsTimeout=0;
    endServiceMsg = new cMessage("end-service");
    contextSwitchMsg = new cMessage("context-switch");
    timeoutMsg = new cMessage("timeout");
    fifo = par("fifo");
    capacity = par("capacity");
    timeSlice = par("timeSlice");
    queue.setName("queue");
    maxServiceTime=par("maxServiceTime");
    timeout=par("timeout");
}

void FogPUTS::scheduleNextEvent() {
    // Schedule context switch or end of service for current job
    simtime_t remainingTime, now;
    now=simTime();
    remainingTime=getRemainingTime(jobServiced);
    //EV << "scheduleNextEvent (remainingTime=" << remainingTime << "s, jobId=" << jobServiced->getId()<<")\n";
    if ((remainingTime <= timeSlice)||(timeSlice <= 0)) {
        setRemainingTime(jobServiced, remainingTime);
        scheduleAt(now+(remainingTime*congestionMultiplier), endServiceMsg );
        //EV << "will schedule endService\n";
    } else {
        setRemainingTime(jobServiced, remainingTime-timeSlice);
        scheduleAt(now+(timeSlice*congestionMultiplier), contextSwitchMsg );
        //EV << "will schedule contextSwitch\n";
    }
}

void FogPUTS::processCloudCongestionUpdateMessage(cMessage *msg){
    CloudCongestionUpdate *umsg=check_and_cast<CloudCongestionUpdate *>(msg);
    double oldCongestionMultiplier=congestionMultiplier;
    EV << "processCloudCongestionUpdateMessage(cong=" << umsg->getMultiplier() << ")\n";
    simtime_t remainingTime=0.0;
    simtime_t now;
    now=simTime();
    if (congested){
        totalCongestionTime=totalCongestionTime + (now-startCongestionTime);
    } else {
        totalNCongestionTime=totalNCongestionTime + (now-startNCongestionTime);
    }
    congested=umsg->getCongestion();
    if (congested){
        congestionMultiplier=umsg->getMultiplier();
        startCongestionTime=now;
    } else {
        congestionMultiplier=1.0;
        startNCongestionTime=now;
    }
    // dispose of congestionUpdate message
    delete umsg;
    if (busy){
        if (endServiceMsg->isScheduled()){
            remainingTime=endServiceMsg->getArrivalTime() - now;
            cancelEvent(endServiceMsg);
            scheduleAt(now+(remainingTime * congestionMultiplier/oldCongestionMultiplier),
                    endServiceMsg);
        }
        if (contextSwitchMsg->isScheduled()){
            remainingTime=contextSwitchMsg->getArrivalTime() - now;
            cancelEvent(contextSwitchMsg);
            scheduleAt(now+(remainingTime * congestionMultiplier/oldCongestionMultiplier),
                    contextSwitchMsg);
        }
    }
}

void FogPUTS::processContextSwitchMessage(cMessage *msg){
    EV << "processContextSwitchMessage " << msg << "(job serviced="<< jobServiced->getId() <<", qlen="<< queue.getLength() <<")\n";
    removeExpiredJobs();
    if (!queue.isEmpty()){
        FogJob *oldJob;
        // stop service of current job
        stopService(jobServiced);
        // context switch
        oldJob=jobServiced;
        jobServiced = getFromQueue();
        queue.insert(oldJob);
        // resume service of new job
        resumeService(jobServiced);
    }
    //EV << "processContextSwitchMessage (remainingTime=" << getRemainingTime(jobServiced) << " job serviced " << jobServiced->getId()  << ")\n";
    // service
    scheduleNextEvent();
}


void FogPUTS::processEndServiceMessage(cMessage *msg){
    EV << "processEndServiceMessage " << msg << " (job serviced " << jobServiced->getId() <<")\n";
    removeExpiredJobs();
    endService(jobServiced);
    if (queue.isEmpty()) {
        jobServiced = NULL;
        //emit(busySignal, 0);
        changeState(ANY2IDLE);
    } else {
        jobServiced = getFromQueue();
        //emit(queueLengthSignal, length());
        resumeService( jobServiced );
        EV << "processEndAppJobMessage: (new service time=" << getRemainingTime(jobServiced) << " new job is " << jobServiced->getId()  << ")\n";
        changeState(ANY2BUSY);
        scheduleNextEvent();
    }
}

void FogPUTS::processCloudAppJobMessage(cMessage *msg){
    EV << "processCloudAppJobMessage " << msg << "\n";
    FogJob *job = check_and_cast<FogJob *>(msg);
    setupService(job);
    // if timeout is expired, simply drop the job and continue
    if (checkTimeoutExpired(job)){
        return;
    }
    totalJobs++;
    if (!jobServiced) {
        // processor was idle
        jobServiced = job;
        //emit(busySignal, 1);
        changeState(ANY2BUSY);
        EV << "processCloudAppJobMessage: service=" << getRemainingTime(jobServiced) << " job is " << jobServiced->getId()  << "\n";
        resumeService(jobServiced);
        scheduleNextEvent();
    } else {
        // check for container capacity
        if (capacity >=0 && queue.getLength() >= capacity) {
            EV << "Capacity full! Job dropped.\n";
            //if (ev.isGUI()) bubble("Dropped!");
            //emit(droppedSignal, 1);
            droppedJobsQueue++;
            delete job;
            return;
        }
        queue.insert( job );
        //emit(queueLengthSignal, length());
        if (timeSlice <= 0) {job->setQueueCount(job->getQueueCount() + 1);}
    }
}

void FogPUTS::processTimeoutMessage(cMessage *msg){
    EV << "Got Timeout Event";
    // drop current job
    if (checkTimeoutExpired(jobServiced)){
        // select new job
        removeExpiredJobs();
        if (queue.isEmpty()) {
            jobServiced = NULL;
            //emit(busySignal, 0);
            changeState(ANY2IDLE);
        } else {
            jobServiced = getFromQueue();
            //emit(queueLengthSignal, length());
            resumeService( jobServiced );
            EV << "processEndAppJobMessage: (new service time=" << getRemainingTime(jobServiced) << " new job is " << jobServiced->getId()  << ")\n";
            changeState(ANY2BUSY);
            scheduleNextEvent();
        }
    }
}

void FogPUTS::setRemainingTime(cMessage *job, simtime_t time){
    cMsgPar *par;
    if (job->hasPar(REMAININGTIME)) {
        par=&(job->par(REMAININGTIME));
        par->setDoubleValue(SIMTIME_DBL(time));
    } else {
        par=new cMsgPar(REMAININGTIME);
        par->setDoubleValue(SIMTIME_DBL(time));
        job->addPar(par);
    }
}

simtime_t FogPUTS::getRemainingTime(cMessage *job){
    cMsgPar par;
    if (job->hasPar(REMAININGTIME)) {
        par=job->par(REMAININGTIME);
        return (double) par;
    } else {
        return 0;
    }
}


void FogPUTS::deleteRemainingTime(cMessage *job){
    // remove parameter
    cObject * p=jobServiced->removeObject(REMAININGTIME);
    if (p!=NULL){delete p;}

}

void FogPUTS::handleMessage(cMessage *msg) {
    //EV << "handling message " << msg;
    //if (endServiceMsg->isScheduled()) {EV << " endSch=true";}
    //if (contextSwitchMsg->isScheduled()) {EV << " ctxSch=true";}
    //EV << "\n";
    if (msg==timeoutMsg) {
        processTimeoutMessage(msg);
    } else {
        if (msg==endServiceMsg) {
            processEndServiceMessage(msg);
        } else {
            if (msg==contextSwitchMsg) {
                processContextSwitchMessage(msg);
            } else {
                if (strcmp(msg->getArrivalGate()->getName(), CONGESTIONGATENAME)==0) {
                    processCloudCongestionUpdateMessage(msg);
                } else {
                    processCloudAppJobMessage(msg);
                }
            }
        }
    }
    //if (ev.isGUI()) getDisplayString().setTagArg("i",1, !jobServiced ? "" : "cyan3");
}

FogJob *FogPUTS::getFromQueue() {
    // we assume that no expired jobs are present in the list
    FogJob *job;
    do {
        if (fifo) {
            job = (FogJob *)queue.pop();
        } else {
            job = (FogJob *)queue.back();
            // FIXME this may have bad performance as remove uses linear search
            queue.remove(job);
        }
    } while (checkTimeoutExpired(job));
    return job;
}

int FogPUTS::length() {
    return queue.getLength();
}

simtime_t FogPUTS::setupService(FogJob *job) {
    // gather initial queueing time statistics
    simtime_t t;
    //EV << "Setting up service of " << job->getId() << endl;
    job->setTimestamp();
    t=par("serviceTime").doubleValue();
    if (maxServiceTime>0 && t>maxServiceTime){ t=maxServiceTime; }
    setRemainingTime(job, t);
    return t;
}

void FogPUTS::endService(FogJob *job) {
    //EV << "Finishing service of " << job->getName() << endl;
    //simtime_t d = simTime() - job->getTimestamp();
    //simtime_t d=stopService(job);
    stopService(job);
    //job->setServiceTime(job->getServiceTime() + d);
    deleteRemainingTime(job);
    cancelTimeout(job);
    if (! checkTimeoutExpired(job)){
        send(job, "out");
    }
}

void FogPUTS::resumeService(FogJob *job) {
    simtime_t now, ts, d;
    now=simTime();
    ts=job->getTimestamp();
    d=now-ts;
    job->setQueuingTime(job->getQueuingTime() + d);
    job->setTimestamp();
    setTimeout(job);
}

void FogPUTS::stopService(FogJob *job) {
    simtime_t now, ts, d;
    now=simTime();
    ts=job->getTimestamp();
    d=now-ts;
    job->setServiceTime(job->getServiceTime() + d);
    job->setTimestamp();
    cancelTimeout(job);
}

bool FogPUTS::checkTimeoutExpired(FogJob *job, bool autoremove){
    if (job==NULL){return false;}
    simtime_t now=simTime();
    if ((timeout>0) && (now - job->getStartTime()>timeout)){
        if (autoremove){
            //EV << "Dropping job from checkTimeoutExpired()";
            // drop and increase droppedJobTimeout
            droppedJobsTimeout++;
            delete job;
        }
        return true;
    }
    return false;
}

void FogPUTS::setTimeout(FogJob *job){
    // timeout can occur only for the currently processed job
    simtime_t to;
    if (timeout<=0)
        return;
    // compute timeout for current job
    to=job->getStartTime()+timeout;
    // set timeout
    scheduleAt(to, timeoutMsg);
}

void FogPUTS::cancelTimeout(FogJob *job){
    if (timeoutMsg->isScheduled()){
        cancelEvent(timeoutMsg);
    }
}

void FogPUTS::removeExpiredJobs(){
    // iterate over queue
    //EV << "FogPUTS::removeExpiredJobs()..."<< endl;
    for (cQueue::Iterator i(queue); !i.end(); ++i){
        //CloudAppJob *job=check_and_cast<CloudAppJob *>((*i)++);
        //CloudAppJob *job=check_and_cast<CloudAppJob *>(*i);
        FogJob *job=check_and_cast<FogJob *>(*i);
        // if job is expired the job is removed
        if (checkTimeoutExpired(job, false)){
            //EV << "removing job" <<endl;
            queue.remove(job);
            // drop and increase droppedJobTimeout
            droppedJobsTimeout++;
            delete job;
        }
    }
    //EV << "done."<< endl;
}


void FogPUTS::changeState(int transition){
    simtime_t now;
    switch (transition){
        case BUSY2IDLE:
            EV << "Busy -> Idle" << endl;
            now=simTime();
            busy=false;
            startIdleTime=now;
            totalBusyTime += now-startBusyTime;
            break;
        case IDLE2BUSY:
            EV << "Idle -> Busy" << endl;
            now=simTime();
            busy=true;
            startBusyTime=now;
            totalIdleTime += now-startIdleTime;
            break;
        case BUSY2BUSY:
            // nothing to do
            break;
        case IDLE2IDLE:
            // nothing to do
            break;
        case ANY2BUSY:
            if (!busy){changeState(IDLE2BUSY);} // else {changeState(IDLE2IDLE);}
            break;
        case ANY2IDLE:
            if (busy){changeState(BUSY2IDLE);} // else {changeState(BUSY2BUSY);}
            break;
    }
}


void FogPUTS::finish()
{
    float rho, congRatio;
    simtime_t now;
    now=simTime();
    // server utilization
    if (busy){
        totalBusyTime += now-startBusyTime;
    } else {
        totalIdleTime += now-startIdleTime;
    }
    rho=totalBusyTime/(totalBusyTime+totalIdleTime);
    recordScalar("rho", rho);
    // congestion ratio
    if (congested){
        totalCongestionTime += now-startCongestionTime;
    } else {
        totalNCongestionTime += now-startNCongestionTime;
    }
    congRatio=totalCongestionTime/(totalCongestionTime+totalNCongestionTime);
    recordScalar("congestionRatio", congRatio);
    // totalJobs, droppedJobs
    recordScalar("totalJobs", totalJobs);
    recordScalar("droppedJobsQueue", droppedJobsQueue);
    recordScalar("droppedJobsTimout", droppedJobsTimeout);
    recordScalar("droppedJobsTotal", droppedJobsQueue+droppedJobsTimeout);
}

}; //namespace

