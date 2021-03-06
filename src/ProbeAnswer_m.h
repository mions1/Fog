//
// Generated file, do not edit! Created by nedtool 5.4 from ProbeAnswer.msg.
//

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#ifndef __FOG_PROBEANSWER_M_H
#define __FOG_PROBEANSWER_M_H

#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0504
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif


namespace fog {

/**
 * Class generated from <tt>ProbeAnswer.msg:22</tt> by nedtool.
 * <pre>
 * //CREATO PER POTER AGGIUNGER LA INFO SULLA QUERYID NELLA RISPOSTA AL PROBE
 * //SE VOGLIAMO MANTENERE SOLO IL LOADUPDATE POSSIAMO AGGIUNGERE LI LA QUERYID
 * //E SETTARLO A -1 SE LA INFO NON CI SERVE
 * message ProbeAnswer
 * {
 *     double load;
 *     int queryId;
 *     int appId;
 * }
 * </pre>
 */
class ProbeAnswer : public ::omnetpp::cMessage
{
  protected:
    double load;
    int queryId;
    int appId;

  private:
    void copy(const ProbeAnswer& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const ProbeAnswer&);

  public:
    ProbeAnswer(const char *name=nullptr, short kind=0);
    ProbeAnswer(const ProbeAnswer& other);
    virtual ~ProbeAnswer();
    ProbeAnswer& operator=(const ProbeAnswer& other);
    virtual ProbeAnswer *dup() const override {return new ProbeAnswer(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual double getLoad() const;
    virtual void setLoad(double load);
    virtual int getQueryId() const;
    virtual void setQueryId(int queryId);
    virtual int getAppId() const;
    virtual void setAppId(int appId);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ProbeAnswer& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ProbeAnswer& obj) {obj.parsimUnpack(b);}

} // namespace fog

#endif // ifndef __FOG_PROBEANSWER_M_H

