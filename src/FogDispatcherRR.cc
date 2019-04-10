//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include "FogDispatcherRR.h"

namespace fog {

Define_Module(FogDispatcherRR);

void FogDispatcherRR::initialize()
{
    setPerformFinalGC(true); // FIXME: WTF?
    currentOut=0;
    nOut=gateSize("out");
    ASSERT(nOut>0);
}

void FogDispatcherRR::handleMessage(cMessage *msg)
{
    send(msg, "out", currentOut);
    currentOut=(currentOut+1)%nOut;
}

}; // namespace
