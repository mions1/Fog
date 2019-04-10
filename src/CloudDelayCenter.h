//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifndef __CLOUDDELAYCENTER_H
#define __CLOUDDELAYCENTER_H

#include "../../queueinglib/QueueingDefs.h"
#include "FogJob_m.h"
#include <map>

typedef std::map<long, cMessage *> jobMap;
typedef std::map<long, cMessage *>::iterator jobMapIterator;

namespace fog {

/**
 * Delays the incoming messages
 */
class CloudDelayCenter : public cSimpleModule
{
    private:
		//simsignal_t delayedJobsSignal;
        int currentlyStored;
        simtime_t maxDelay;
        simtime_t timeout;
        //eventlist: use job->getId() to obtain a unique ID for the message
        jobMap jobs;
        bool congested;
        double congestionMultiplier;
        void processNewCloudAppJob(cMessage *msg);
        void processReturningCloudAppJob(cMessage *msg);
        void processCloudCongestionUpdateMessage(cMessage *msg);
        //void processTimeoutMessage(cMessage *msg);
        bool checkTimeoutExpired(FogJob *job, bool autoremove=true);
        int totalJobs;
        int droppedJobsTimeout;

    protected:
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
        virtual void finish();

};

}; //namespace

#endif

