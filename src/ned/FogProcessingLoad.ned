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
module FogProcessingLoad
{
    parameters:
        @display("i=block/network;bgb=541,230;bgl=5");
        int appId;
        int nServers;
        int nApps;
        double timeout @unit(s) = default(-1s);
        @display("bgb=893,271");

    gates:
        output out;
        output toNodes[nApps-1];
        input fromNodes[nApps-1];

    submodules:
        source: FogSource {
            @display("p=490,109");
        }
        server[nServers]: FogPU {
            @display("p=207,109");
        }
        cloudAppDispatcherLoad: FogDispatcherLoad {
            @display("p=354,109");
        }
        merge: Merge {
            @display("p=67,109");
        }
    connections allowunconnected:
        for i=0..nServers-1 {
            server[i].out --> merge.in++;
            server[i].loadUpdate --> cloudAppDispatcherLoad.localLoad++;
            cloudAppDispatcherLoad.out++ --> server[i].in++;

        }
        for j=0..nApps-2 {
            cloudAppDispatcherLoad.probeOuterLoad++ --> toNodes[j];
            fromNodes[j] --> cloudAppDispatcherLoad.outerLoad++;
        }
        merge.out --> out;
        source.out --> cloudAppDispatcherLoad.in++;

}
