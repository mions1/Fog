//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package fog;

import org.omnetpp.queueing.Merge;


//
// A subset of VMs processing requests of the same app
//
module FogNode
{
    parameters:
        @display("i=block/network");
        int nApps;
        int nServers;
        server[*].maxServiceTime = default(-1s);
        dispatcher.nApps=nApps;
       	//server.fifo=true
       	//server.capacity=-1
    gates:
        output out;
        input in[];
        output toNeighbors[];
        input fromNeighbors[];
    submodules:
        dispatcher: FogLoadBalancer;
        server[nServers]: FogPU;
        merge: Merge;
    connections allowunconnected:
        in++ --> dispatcher.in++;
        dispatcher.loadToNeighbor++ --> toNeighbors++;
        fromNeighbors++ --> dispatcher.loadFromNeighbor++;
        for i=0..nServers-1 {
            dispatcher.out++ --> server[i].in++;
            server[i].loadUpdate --> dispatcher.localLoad++;
            server[i].out --> merge.in++;
        }
        merge.out --> out;
}
