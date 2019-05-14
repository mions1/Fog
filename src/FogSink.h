//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __CLOUDSINK_H
#define __CLOUDSINK_H

#include <omnetpp.h>
#include <vector>
#include <map>

using namespace omnetpp;

namespace fog {

/**
 * Message sink; see NED file for more info.
 */
class FogSink : public cSimpleModule
{
  private:
    // statistics
    std::map<int, cOutVector> responseTimeVector;
    std::map<int, cOutVector> delayTimeVector;
    std::map<int, cOutVector> queuingTimeVector;
    std::map<int, cOutVector> serviceTimeVector;
    std::map<int, cOutVector> balancerTimeVector;
    std::map<int, cOutVector> balancerCountVector;
    std::map<int, cPSquare> responseTimeStat;
    std::map<int, cPSquare> delayTimeStat;
    std::map<int, cPSquare> queuingTimeStat;
    std::map<int, cPSquare> serviceTimeStat;
    std::map<int, cPSquare> balancerTimeStat;
    double get90percentile(cAbstractHistogram *);
    void dumpStat(std::map<int, cPSquare> *, std::string);
    // FIXME: linking error in this function
    void initVector(std::map<int, cOutVector> *, int, std::string);

  public:
    virtual ~FogSink();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

}; // namespace

#endif
