//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//
#ifndef __CLOUDAPPDISPATCHERRR_H
#define __CLOUDAPPDISPATCHERRR_H

#include <omnetpp.h>

using namespace omnetpp;

namespace fog {

/**
 * Forks a job. See the NED file for more info.
 */
class FogDispatcherRR : public cSimpleModule
{
  private:
    int nOut;
    int currentOut;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

}; // namespace

#endif
