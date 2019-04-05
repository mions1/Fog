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
// A block that delays incoming messages by some time. The delay time is
// determined by the <code>delay</code> parameter which can be a distribution.
// Please note that the order of incoming and outgoing messages are NOT necessarily the same.
//
// @author rhornig
//
simple CloudDelayCenter
{
    parameters:
        //@group(Queueing);
        //@signal[delayedJobs](type="int");
        //@statistic[delayedJobs](title="number of delayed jobs";record=vector?,timeavg,max;interpolationmode=sample-hold);
        @display("i=block/delay");
        volatile double delay @unit(s); // the requested delay time (can be a distribution)
        double maxDelay @unit(s) = default(-1s); // upper bound
        double timeout @unit(s) = default(-1s);
        
    gates:
        input congestionControl;   		// congestion control messages 
        input in[];                     // the incoming message gates
        output out;                     // outgoing message gate
}
