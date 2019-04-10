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

using namespace omnetpp;

namespace fog {

/**
 * Message sink; see NED file for more info.
 */
class CloudSink : public cSimpleModule
{
  private:
    int nApps;
    // statistics
    std::vector<cOutVector *> responseTimeVector;
    std::vector<cOutVector *> delayTimeVector;
    std::vector<cOutVector *> queuingTimeVector;
    std::vector<cOutVector *> serviceTimeVector;
    std::vector<cPSquare *> responseTimeStat;
    std::vector<cPSquare *> delayTimeStat;
    std::vector<cPSquare *> queuingTimeStat;
    std::vector<cPSquare *> serviceTimeStat;
    double get90percentile(cAbstractHistogram *);

  public:
    virtual ~CloudSink();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

}; // namespace

#endif
