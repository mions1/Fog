//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


namespace fog;

//cplusplus {{
//#include "QueueingDefs.h"
//}}

//
// CloudAppJob message definition for M/G/n simulations
//
message FogJob
{
    //@customize(true);
    simtime_t startTime; 		// time the message is sent
    simtime_t queuingTime; 		// total time spent standing in queues
    simtime_t serviceTime;  	// total time spent in servers
    simtime_t delayTime;    	// total time spent in delay modules
    simtime_t balancerTime;    	// total time spent in Load balancing modules
    simtime_t probeTime;		// time passed waiting for probe
    simtime_t slaDeadline;		// deadline for SLA
	simtime_t suggestedTime;	// suggested service time. 
    
    int queueCount;             // the number of queue modules visited by the job
    int delayCount;             // the number of delay modules visited by the job
    int balancerCount;			// the number of load balancing modules visited by the job
    int appId;					// ID of the considered cloud App
    int id;						// id of the msg
    // FIXME: should be bool rather than int
    bool realTime;				// flag realtime, if 1-> drop job when SLA expires, if 0-> no drop
	bool multiHop;				// flag multiHop, if 1-> job can be forwarded
}

