//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package ned;

import org.omnetpp.queueing.Merge;


//
// A subset of VMs processing requests of the same app
//
module FogProcessing
{
    parameters:
        @display("i=block/network");
        int appId;
        int nServers;
        // source parameters
        source.appId = appId;
        source.sendInterval = default(exponential(1s));
        source.maxInterval = default(-1s);
        source.realTime = default(0);
        // server parameters
        server[*].serviceTime = default(exponential(0.9s));
        server[*].maxServiceTime = default(-1s);
       	//server.fifo=true
       	//server.capacity=-1
    gates:
        output out;
    submodules:
        source: FogSource;
        // FIXME: should add also
        server[nServers]: FogPUTS;
        dispatcher: FogDispatcherRR;
        merge: Merge;
    connections:
        source.out --> dispatcher.in;
        for i=0..nServers-1 {
            dispatcher.out++ --> server[i].in++;
            server[i].out --> merge.in++;
        }
        merge.out --> out;
}
