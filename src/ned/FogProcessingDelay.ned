//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package ned;

//
// A subset of VMs processing requests of the same app
//
module FogProcessingDelay
{
    parameters:
        @display("i=block/network");
        int appId;
        int nServers;
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
        delay.timeout = timeout;
       	//server.fifo=true
       	//server.capacity=-1
    gates:
        output out;
    submodules:
        source: FogSource;
        // FIXME: should add also
        server[nServers]: FogPUTS;
        dispatcher: FogDispatcherRR;
        delay: CloudDelayCenter;
        congCtl: CloudCongestionControl;
    connections:
        source.out --> dispatcher.in;
        for i=0..nServers-1 {
            dispatcher.out++ --> server[i].in++;
            server[i].out --> delay.in++;
            congCtl.congestionControl[i] --> server[i].congestionControl;
        }
        delay.out --> out;
}
