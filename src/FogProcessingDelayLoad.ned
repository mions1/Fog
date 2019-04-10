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
module FogProcessingDelayLoad
{
    parameters:
        @display("i=block/network;bgb=483.16,317.72");
        int appId;
        int nServers;
        int nApps;
        double timeout @unit(s) = default(-1s);
        // source parameters
        source.appId = appId;
        source.sendInterval = default(exponential(1s));
        source.maxInterval = default(-1s);
        // server parameters
        server[*].serviceTime = default(exponential(0.9s));
        server[*].maxServiceTime = default(-1s);
        server[*].timeout = timeout;
        // delay parameters
        delay[*].timeout = timeout;
       	//server.fifo=true
       	//server.capacity=-1
    gates:
        output out;
        output toNodes[nApps-1];
        input fromNodes[nApps-1];

    submodules:
        source: FogSource {
            @display("p=363.78,102.46");
        }
        // FIXME: should add also
        server[nServers]: FogPU {
            @display("p=69,81");
        }
        delay[nServers]: CloudDelayCenter {
            @display("p=160.74,188");
        }
        congCtl[nServers]: CloudCongestionControl {
            @display("p=283.88,188");
        }
        merge: Merge;
        cloudAppDispatcherLoad: FogDispatcherLoad {
            @display("p=247.22,95.88");
        }
    connections allowunconnected:
        for i=0..nServers-1 {
            server[i].out --> delay[i].in++;
            congCtl[i].congestionControl++ --> server[i].congestionControl;
            congCtl[i].congestionControl++ --> delay[i].congestionControl;
            delay[i].out --> merge.in++;
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
