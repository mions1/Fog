//
// Generated file, do not edit! Created by nedtool 5.4 from FogJob.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include "FogJob_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace fog {

// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

Register_Class(FogJob)

FogJob::FogJob(const char *name, short kind) : ::omnetpp::cMessage(name,kind)
{
    this->startTime = 0;
    this->queuingTime = 0;
    this->serviceTime = 0;
    this->delayTime = 0;
    this->balancerTime = 0;
    this->probeTime = 0;
    this->slaDeadline = 0;
    this->suggestedTime = 0;
    this->queueCount = 0;
    this->delayCount = 0;
    this->balancerCount = 0;
    this->appId = 0;
    this->id = 0;
    this->realTime = false;
    this->multiHop = false;
}

FogJob::FogJob(const FogJob& other) : ::omnetpp::cMessage(other)
{
    copy(other);
}

FogJob::~FogJob()
{
}

FogJob& FogJob::operator=(const FogJob& other)
{
    if (this==&other) return *this;
    ::omnetpp::cMessage::operator=(other);
    copy(other);
    return *this;
}

void FogJob::copy(const FogJob& other)
{
    this->startTime = other.startTime;
    this->queuingTime = other.queuingTime;
    this->serviceTime = other.serviceTime;
    this->delayTime = other.delayTime;
    this->balancerTime = other.balancerTime;
    this->probeTime = other.probeTime;
    this->slaDeadline = other.slaDeadline;
    this->suggestedTime = other.suggestedTime;
    this->queueCount = other.queueCount;
    this->delayCount = other.delayCount;
    this->balancerCount = other.balancerCount;
    this->appId = other.appId;
    this->id = other.id;
    this->realTime = other.realTime;
    this->multiHop = other.multiHop;
}

void FogJob::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cMessage::parsimPack(b);
    doParsimPacking(b,this->startTime);
    doParsimPacking(b,this->queuingTime);
    doParsimPacking(b,this->serviceTime);
    doParsimPacking(b,this->delayTime);
    doParsimPacking(b,this->balancerTime);
    doParsimPacking(b,this->probeTime);
    doParsimPacking(b,this->slaDeadline);
    doParsimPacking(b,this->suggestedTime);
    doParsimPacking(b,this->queueCount);
    doParsimPacking(b,this->delayCount);
    doParsimPacking(b,this->balancerCount);
    doParsimPacking(b,this->appId);
    doParsimPacking(b,this->id);
    doParsimPacking(b,this->realTime);
    doParsimPacking(b,this->multiHop);
}

void FogJob::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->startTime);
    doParsimUnpacking(b,this->queuingTime);
    doParsimUnpacking(b,this->serviceTime);
    doParsimUnpacking(b,this->delayTime);
    doParsimUnpacking(b,this->balancerTime);
    doParsimUnpacking(b,this->probeTime);
    doParsimUnpacking(b,this->slaDeadline);
    doParsimUnpacking(b,this->suggestedTime);
    doParsimUnpacking(b,this->queueCount);
    doParsimUnpacking(b,this->delayCount);
    doParsimUnpacking(b,this->balancerCount);
    doParsimUnpacking(b,this->appId);
    doParsimUnpacking(b,this->id);
    doParsimUnpacking(b,this->realTime);
    doParsimUnpacking(b,this->multiHop);
}

::omnetpp::simtime_t FogJob::getStartTime() const
{
    return this->startTime;
}

void FogJob::setStartTime(::omnetpp::simtime_t startTime)
{
    this->startTime = startTime;
}

::omnetpp::simtime_t FogJob::getQueuingTime() const
{
    return this->queuingTime;
}

void FogJob::setQueuingTime(::omnetpp::simtime_t queuingTime)
{
    this->queuingTime = queuingTime;
}

::omnetpp::simtime_t FogJob::getServiceTime() const
{
    return this->serviceTime;
}

void FogJob::setServiceTime(::omnetpp::simtime_t serviceTime)
{
    this->serviceTime = serviceTime;
}

::omnetpp::simtime_t FogJob::getDelayTime() const
{
    return this->delayTime;
}

void FogJob::setDelayTime(::omnetpp::simtime_t delayTime)
{
    this->delayTime = delayTime;
}

::omnetpp::simtime_t FogJob::getBalancerTime() const
{
    return this->balancerTime;
}

void FogJob::setBalancerTime(::omnetpp::simtime_t balancerTime)
{
    this->balancerTime = balancerTime;
}

::omnetpp::simtime_t FogJob::getProbeTime() const
{
    return this->probeTime;
}

void FogJob::setProbeTime(::omnetpp::simtime_t probeTime)
{
    this->probeTime = probeTime;
}

::omnetpp::simtime_t FogJob::getSlaDeadline() const
{
    return this->slaDeadline;
}

void FogJob::setSlaDeadline(::omnetpp::simtime_t slaDeadline)
{
    this->slaDeadline = slaDeadline;
}

::omnetpp::simtime_t FogJob::getSuggestedTime() const
{
    return this->suggestedTime;
}

void FogJob::setSuggestedTime(::omnetpp::simtime_t suggestedTime)
{
    this->suggestedTime = suggestedTime;
}

int FogJob::getQueueCount() const
{
    return this->queueCount;
}

void FogJob::setQueueCount(int queueCount)
{
    this->queueCount = queueCount;
}

int FogJob::getDelayCount() const
{
    return this->delayCount;
}

void FogJob::setDelayCount(int delayCount)
{
    this->delayCount = delayCount;
}

int FogJob::getBalancerCount() const
{
    return this->balancerCount;
}

void FogJob::setBalancerCount(int balancerCount)
{
    this->balancerCount = balancerCount;
}

int FogJob::getAppId() const
{
    return this->appId;
}

void FogJob::setAppId(int appId)
{
    this->appId = appId;
}

int FogJob::getId() const
{
    return this->id;
}

void FogJob::setId(int id)
{
    this->id = id;
}

bool FogJob::getRealTime() const
{
    return this->realTime;
}

void FogJob::setRealTime(bool realTime)
{
    this->realTime = realTime;
}

bool FogJob::getMultiHop() const
{
    return this->multiHop;
}

void FogJob::setMultiHop(bool multiHop)
{
    this->multiHop = multiHop;
}

class FogJobDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    FogJobDescriptor();
    virtual ~FogJobDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(FogJobDescriptor)

FogJobDescriptor::FogJobDescriptor() : omnetpp::cClassDescriptor("fog::FogJob", "omnetpp::cMessage")
{
    propertynames = nullptr;
}

FogJobDescriptor::~FogJobDescriptor()
{
    delete[] propertynames;
}

bool FogJobDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<FogJob *>(obj)!=nullptr;
}

const char **FogJobDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *FogJobDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int FogJobDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 15+basedesc->getFieldCount() : 15;
}

unsigned int FogJobDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<15) ? fieldTypeFlags[field] : 0;
}

const char *FogJobDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "startTime",
        "queuingTime",
        "serviceTime",
        "delayTime",
        "balancerTime",
        "probeTime",
        "slaDeadline",
        "suggestedTime",
        "queueCount",
        "delayCount",
        "balancerCount",
        "appId",
        "id",
        "realTime",
        "multiHop",
    };
    return (field>=0 && field<15) ? fieldNames[field] : nullptr;
}

int FogJobDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "startTime")==0) return base+0;
    if (fieldName[0]=='q' && strcmp(fieldName, "queuingTime")==0) return base+1;
    if (fieldName[0]=='s' && strcmp(fieldName, "serviceTime")==0) return base+2;
    if (fieldName[0]=='d' && strcmp(fieldName, "delayTime")==0) return base+3;
    if (fieldName[0]=='b' && strcmp(fieldName, "balancerTime")==0) return base+4;
    if (fieldName[0]=='p' && strcmp(fieldName, "probeTime")==0) return base+5;
    if (fieldName[0]=='s' && strcmp(fieldName, "slaDeadline")==0) return base+6;
    if (fieldName[0]=='s' && strcmp(fieldName, "suggestedTime")==0) return base+7;
    if (fieldName[0]=='q' && strcmp(fieldName, "queueCount")==0) return base+8;
    if (fieldName[0]=='d' && strcmp(fieldName, "delayCount")==0) return base+9;
    if (fieldName[0]=='b' && strcmp(fieldName, "balancerCount")==0) return base+10;
    if (fieldName[0]=='a' && strcmp(fieldName, "appId")==0) return base+11;
    if (fieldName[0]=='i' && strcmp(fieldName, "id")==0) return base+12;
    if (fieldName[0]=='r' && strcmp(fieldName, "realTime")==0) return base+13;
    if (fieldName[0]=='m' && strcmp(fieldName, "multiHop")==0) return base+14;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *FogJobDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "simtime_t",
        "simtime_t",
        "simtime_t",
        "simtime_t",
        "simtime_t",
        "simtime_t",
        "simtime_t",
        "simtime_t",
        "int",
        "int",
        "int",
        "int",
        "int",
        "bool",
        "bool",
    };
    return (field>=0 && field<15) ? fieldTypeStrings[field] : nullptr;
}

const char **FogJobDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *FogJobDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int FogJobDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    FogJob *pp = (FogJob *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *FogJobDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    FogJob *pp = (FogJob *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string FogJobDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    FogJob *pp = (FogJob *)object; (void)pp;
    switch (field) {
        case 0: return simtime2string(pp->getStartTime());
        case 1: return simtime2string(pp->getQueuingTime());
        case 2: return simtime2string(pp->getServiceTime());
        case 3: return simtime2string(pp->getDelayTime());
        case 4: return simtime2string(pp->getBalancerTime());
        case 5: return simtime2string(pp->getProbeTime());
        case 6: return simtime2string(pp->getSlaDeadline());
        case 7: return simtime2string(pp->getSuggestedTime());
        case 8: return long2string(pp->getQueueCount());
        case 9: return long2string(pp->getDelayCount());
        case 10: return long2string(pp->getBalancerCount());
        case 11: return long2string(pp->getAppId());
        case 12: return long2string(pp->getId());
        case 13: return bool2string(pp->getRealTime());
        case 14: return bool2string(pp->getMultiHop());
        default: return "";
    }
}

bool FogJobDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    FogJob *pp = (FogJob *)object; (void)pp;
    switch (field) {
        case 0: pp->setStartTime(string2simtime(value)); return true;
        case 1: pp->setQueuingTime(string2simtime(value)); return true;
        case 2: pp->setServiceTime(string2simtime(value)); return true;
        case 3: pp->setDelayTime(string2simtime(value)); return true;
        case 4: pp->setBalancerTime(string2simtime(value)); return true;
        case 5: pp->setProbeTime(string2simtime(value)); return true;
        case 6: pp->setSlaDeadline(string2simtime(value)); return true;
        case 7: pp->setSuggestedTime(string2simtime(value)); return true;
        case 8: pp->setQueueCount(string2long(value)); return true;
        case 9: pp->setDelayCount(string2long(value)); return true;
        case 10: pp->setBalancerCount(string2long(value)); return true;
        case 11: pp->setAppId(string2long(value)); return true;
        case 12: pp->setId(string2long(value)); return true;
        case 13: pp->setRealTime(string2bool(value)); return true;
        case 14: pp->setMultiHop(string2bool(value)); return true;
        default: return false;
    }
}

const char *FogJobDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *FogJobDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    FogJob *pp = (FogJob *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

} // namespace fog

