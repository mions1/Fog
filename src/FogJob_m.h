//
// Generated file, do not edit! Created by nedtool 5.4 from FogJob.msg.
//

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#ifndef __FOG_FOGJOB_M_H
#define __FOG_FOGJOB_M_H

#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0504
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif


namespace fog {

/**
 * Class generated from <tt>FogJob.msg:21</tt> by nedtool.
 * <pre>
 * //
 * // CloudAppJob message definition for M/G/n simulations
 * //
 * message FogJob
 * {
 *     //\@customize(true);
 *     simtime_t startTime; 		// time the message is sent
 *     simtime_t queuingTime; 		// total time spent standing in queues
 *     simtime_t serviceTime;  	// total time spent in servers
 *     simtime_t delayTime;    	// total time spent in delay modules
 *     simtime_t balancerTime;    	// total time spent in Load balancing modules
 *     simtime_t probeTime;		// time passed waiting for probe
 *     simtime_t slaDeadline;		// deadline for SLA
 *     simtime_t suggestedTime;	// suggested service time. 
 * 
 *     int queueCount;             // the number of queue modules visited by the job
 *     int delayCount;             // the number of delay modules visited by the job
 *     int balancerCount;			// the number of load balancing modules visited by the job
 *     int appId;					// ID of the considered cloud App
 *     int id;						// id of the msg
 *     // FIXME: should be bool rather than int
 *     bool realTime;				// flag realtime, if 1-> drop job when SLA expires, if 0-> no drop
 *     bool multiHop;				// flag multiHop, if 1-> job can be forwarded
 * }
 * </pre>
 */
class FogJob : public ::omnetpp::cMessage
{
  protected:
    ::omnetpp::simtime_t startTime;
    ::omnetpp::simtime_t queuingTime;
    ::omnetpp::simtime_t serviceTime;
    ::omnetpp::simtime_t delayTime;
    ::omnetpp::simtime_t balancerTime;
    ::omnetpp::simtime_t probeTime;
    ::omnetpp::simtime_t slaDeadline;
    ::omnetpp::simtime_t suggestedTime;
    int queueCount;
    int delayCount;
    int balancerCount;
    int appId;
    int id;
    bool realTime;
    bool multiHop;

  private:
    void copy(const FogJob& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const FogJob&);

  public:
    FogJob(const char *name=nullptr, short kind=0);
    FogJob(const FogJob& other);
    virtual ~FogJob();
    FogJob& operator=(const FogJob& other);
    virtual FogJob *dup() const override {return new FogJob(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual ::omnetpp::simtime_t getStartTime() const;
    virtual void setStartTime(::omnetpp::simtime_t startTime);
    virtual ::omnetpp::simtime_t getQueuingTime() const;
    virtual void setQueuingTime(::omnetpp::simtime_t queuingTime);
    virtual ::omnetpp::simtime_t getServiceTime() const;
    virtual void setServiceTime(::omnetpp::simtime_t serviceTime);
    virtual ::omnetpp::simtime_t getDelayTime() const;
    virtual void setDelayTime(::omnetpp::simtime_t delayTime);
    virtual ::omnetpp::simtime_t getBalancerTime() const;
    virtual void setBalancerTime(::omnetpp::simtime_t balancerTime);
    virtual ::omnetpp::simtime_t getProbeTime() const;
    virtual void setProbeTime(::omnetpp::simtime_t probeTime);
    virtual ::omnetpp::simtime_t getSlaDeadline() const;
    virtual void setSlaDeadline(::omnetpp::simtime_t slaDeadline);
    virtual ::omnetpp::simtime_t getSuggestedTime() const;
    virtual void setSuggestedTime(::omnetpp::simtime_t suggestedTime);
    virtual int getQueueCount() const;
    virtual void setQueueCount(int queueCount);
    virtual int getDelayCount() const;
    virtual void setDelayCount(int delayCount);
    virtual int getBalancerCount() const;
    virtual void setBalancerCount(int balancerCount);
    virtual int getAppId() const;
    virtual void setAppId(int appId);
    virtual int getId() const;
    virtual void setId(int id);
    virtual bool getRealTime() const;
    virtual void setRealTime(bool realTime);
    virtual bool getMultiHop() const;
    virtual void setMultiHop(bool multiHop);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const FogJob& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, FogJob& obj) {obj.parsimUnpack(b);}

} // namespace fog

#endif // ifndef __FOG_FOGJOB_M_H

