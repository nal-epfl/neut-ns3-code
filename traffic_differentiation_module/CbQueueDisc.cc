//
// Created by Zeinab Shmeis on 16.04.20.
//

#include "CbQueueDisc.h"


NS_LOG_COMPONENT_DEFINE("CbQueueDisc");

NS_OBJECT_ENSURE_REGISTERED(CbQueueDisc);

ATTRIBUTE_HELPER_CPP (DscpMap);

std::ostream &operator << (std::ostream &os, const DscpMap &dscpMap) {
    std::copy (dscpMap.begin (), dscpMap.end () - 1, std::ostream_iterator<uint8_t>(os, " "));
    os << dscpMap.back ();
    return os;
}

std::istream &operator >> (std::istream &is, DscpMap &dscpMap) {
    uint8_t val;
    while (is >> val) {
        dscpMap.push_back(val);
    }
    return is;
}

TypeId CbQueueDisc::GetTypeId () {
    static TypeId tid = TypeId ("ns3::CbQueueDisc")
            .SetParent<QueueDisc> ()
            .SetGroupName ("TrafficControl")
            .AddConstructor<CbQueueDisc> ()
            .AddAttribute ("MaxSize",
                           "The max queue size",
                           QueueSizeValue (QueueSize ("1000p")),
                           MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                                  &QueueDisc::GetMaxSize),
                           MakeQueueSizeChecker ())
            .AddAttribute ("DscpMap",
                           "The different type service to classify based on",
                           DscpMapValue(DscpMap{0}),
                           MakeDscpMapAccessor(&CbQueueDisc::SetDscpMap),
                           MakeDscpMapChecker())
    ;
    return tid;
}

CbQueueDisc::CbQueueDisc() : QueueDisc (QueueDiscSizePolicy::SINGLE_CHILD_QUEUE_DISC) {
    NS_LOG_FUNCTION (this);
    _nextBand = 0;
}

CbQueueDisc::~CbQueueDisc() {
    NS_LOG_FUNCTION (this);
}

void CbQueueDisc::DoDispose() {
    NS_LOG_FUNCTION (this);
    QueueDisc::DoDispose ();
}

void CbQueueDisc::SetDscpMap(DscpMap dscpMap) {
    NS_LOG_FUNCTION(this << dscpMap);
    for(int i = 0; i < dscpMap.size(); i++) {
        _dscp2band[dscpMap[i]] = i;
    }
    _nbBands = dscpMap.size();
}

uint16_t CbQueueDisc::Classify(const Ptr<QueueDiscItem>& item) {
    // check what type of service corresponding to this packet
    Ptr<const Ipv4QueueDiscItem> ipItem = DynamicCast<const Ipv4QueueDiscItem>(item);
    Ipv4Header ipHeader = ipItem->GetHeader();
    uint8_t dscp = ipHeader.GetDscp();

    if(_dscp2band.find(dscp) == _dscp2band.end()) { // this dscp is not defined in the map
        return 0;
    }

    return _dscp2band[dscp];
}

bool CbQueueDisc::DoEnqueue(Ptr<QueueDiscItem> item) {
    NS_LOG_FUNCTION(this << item);

    uint16_t band = Classify(item);

    bool retval = GetQueueDiscClass(band)->GetQueueDisc()->Enqueue(item);

    // If Queue::Enqueue fails, QueueDisc::Drop is called by the child queue
    // disc because QueueDisc::AddQueueDiscClass sets the drop callback
    NS_LOG_LOGIC("Current queue size: " << GetNPackets() << " packets, " << GetNBytes() << " bytes");

    return retval;
}

Ptr<QueueDiscItem> CbQueueDisc::DoDequeue() {
    NS_LOG_FUNCTION(this);

    Ptr<QueueDiscItem> item = nullptr;

    for (uint32_t i = 0; i < _nbBands; i++) {
        Ptr<const QueueDiscItem> itemPeek = GetQueueDiscClass(_nextBand)->GetQueueDisc()->Peek();
        if (itemPeek) {
            item = GetQueueDiscClass(_nextBand)->GetQueueDisc()->Dequeue();

            if (!item) {
                NS_LOG_DEBUG ("That's odd! Expecting the peeked packet, we got no packet.");
                return item;
            }
            NS_LOG_LOGIC ("Popped from band " << _nextBand << ": " << item);
            NS_LOG_LOGIC ("Number packets band " << _nextBand << ": "
                                                     << GetQueueDiscClass(_nextBand)->GetQueueDisc()->GetNPackets());
            _nextBand = (_nextBand + 1) % _nbBands;
            return item;
        }
        _nextBand = (_nextBand + 1) % _nbBands;
    }

    NS_LOG_LOGIC ("Queue empty");
    return item;
}

Ptr<const QueueDiscItem> CbQueueDisc::DoPeek() {
    NS_LOG_FUNCTION(this);

    Ptr<const QueueDiscItem> item = nullptr;

    for (uint32_t i = 0; i < _nbBands; i++) {
        uint32_t nextBand = (_nextBand + i) % _nbBands;
        if ((item = GetQueueDiscClass (nextBand)->GetQueueDisc ()->Peek ()) != nullptr)
        {
            NS_LOG_LOGIC ("Peeked from band " << nextBand << ": " << item);
            NS_LOG_LOGIC ("Number packets band " << nextBand << ": " << GetQueueDiscClass (i)->GetQueueDisc ()->GetNPackets ());
            return item;
        }
    }

    NS_LOG_LOGIC ("Queue empty");
    return item;
}

bool CbQueueDisc::CheckConfig() {
    NS_LOG_FUNCTION (this);
    if (GetNInternalQueues () > 0) {
        NS_LOG_ERROR ("CbQueueDisc cannot have internal queues");
        return false;
    }

    if (GetNPacketFilters () > 0) {
        NS_LOG_ERROR ("CbQueueDisc cannot have packet filters");
        return false;
    }

    if (GetNQueueDiscClasses () == 0) {
        // create a FIFO queue disc
        ObjectFactory factory;
        factory.SetTypeId ("ns3::FifoQueueDisc");
        Ptr<QueueDisc> qd = factory.Create<QueueDisc> ();

        if (!qd->SetMaxSize (GetMaxSize ())) {
            NS_LOG_ERROR ("Cannot set the max size of the child queue disc to that of CbQueueDisc");
            return false;
        }
        qd->Initialize ();

        Ptr<QueueDiscClass> c = CreateObject<QueueDiscClass> ();
        c->SetQueueDisc (qd);
        AddQueueDiscClass (c);
    }

    return true;
}

void CbQueueDisc::InitializeParams() {
    NS_LOG_FUNCTION (this);
    _id = EventId ();
}

TrafficControlHelper
CbQueueDisc::GenerateDisc1FifoNPolicers(const string &queueSize, const vector<uint8_t> &childDiscsDscp,
                                        double policingRate, double burstLength, const string& resultsPath) {
    system(("mkdir -p " + resultsPath).c_str());

    TrafficControlHelper policerTch;
    uint16_t handle = policerTch.SetRootQueueDisc("ns3::CbQueueDisc", "MaxSize", StringValue(queueSize), "DscpMap", DscpMapValue(childDiscsDscp));

    TrafficControlHelper::ClassIdList cid = policerTch.AddQueueDiscClasses (handle, childDiscsDscp.size(), "ns3::QueueDiscClass");
    policerTch.AddChildQueueDisc (handle, cid[0], "ns3::FifoQueueDisc", "MaxSize", StringValue(queueSize));

    int burst = floor(policingRate * burstLength * 125000);// in byte
    for (int i = 1; i < childDiscsDscp.size(); i++) {
        policerTch.AddChildQueueDisc(handle, cid[i], "ns3::TbfQueueDiscChild",
                                     "MaxSize", StringValue(to_string(burst) + "B"),
                                     "Burst", UintegerValue(burst),
                                     "Mtu", UintegerValue (1500),
                                     "Rate", DataRateValue(DataRate(to_string(policingRate) + "Mbps")),
                                     "PeakRate", DataRateValue(DataRate("0bps")),
                                     "QueueTraceOutput", StringValue(resultsPath + "/enqueued_events_policer" + to_string(childDiscsDscp[i]) + ".csv"));
    }

    return policerTch;
}
