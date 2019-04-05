//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


namespace fog;


//
// CloudCongestionUpdate message definition for M/G/n simulations
//
message CloudCongestionUpdate
{
    //@customize(true);
    bool congestion;			// Congestion status: true or false
    double multiplier;			// multiplier for service/delay time
}


