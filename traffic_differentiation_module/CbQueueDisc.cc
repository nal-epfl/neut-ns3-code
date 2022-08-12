//
// Created by Zeinab Shmeis on 16.04.20.
//

#include "CbQueueDisc.h"

#include <utility>

NS_LOG_COMPONENT_DEFINE("CbQueueDisc");
NS_OBJECT_ENSURE_REGISTERED(CbQueueDisc);
ATTRIBUTE_HELPER_CPP (TrafficClassifier);

ostream& operator<< (ostream& os, const TrafficClassifier &classifier) {
    vector<Dscps2QueueBand*> dscps2bands = classifier.GetDscps2Bands();
    std::copy (dscps2bands.begin (), dscps2bands.end () - 1, std::ostream_iterator<Dscps2QueueBand*>(os, " "));
    os << dscps2bands.back ();
    return os;
}

istream& operator>> (istream& is, TrafficClassifier &classifier) {
    Dscps2QueueBand val;
    while (is >> val) { classifier.push_back(&val); }
    return is;
}

uint16_t TrafficClassifier::ClassifyDscp(uint8_t dscp) {
    for(auto dscps2band: _dscps2bands) {
        if (dscps2band->CheckIfBelongsTo(dscp)) {
            return dscps2band->GetBand();
        }
    }
    return 0;
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
            .AddAttribute ("ChildQueuesClassifiers",
                           "The different type service to classify based on",
                           TrafficClassifierValue(TrafficClassifier()),
                           MakeTrafficClassifierAccessor(&CbQueueDisc::SetTrafficClassifier),
                           MakeTrafficClassifierChecker())
    ;
    return tid;
}

CbQueueDisc::CbQueueDisc() : QueueDisc (QueueDiscSizePolicy::SINGLE_CHILD_QUEUE_DISC) {
    NS_LOG_FUNCTION (this);
    _nextBand = 0;
    _classifier = TrafficClassifier();
}

CbQueueDisc::~CbQueueDisc() {
    NS_LOG_FUNCTION (this);
}

void CbQueueDisc::SetTrafficClassifier(TrafficClassifier classifier) {
    _classifier = std::move(classifier);
    _nbBands = _classifier.GetNumberOfClasses();
}

void CbQueueDisc::DoDispose() {
    NS_LOG_FUNCTION (this);
    QueueDisc::DoDispose ();
}

uint16_t CbQueueDisc::Classify(const Ptr<QueueDiscItem>& item) {
    // check what type of service corresponding to this packet
    Ptr<const Ipv4QueueDiscItem> ipItem = DynamicCast<const Ipv4QueueDiscItem>(item);
    Ipv4Header ipHeader = ipItem->GetHeader();
    uint8_t dscp = ipHeader.GetDscp();

    uint16_t band = _classifier.ClassifyDscp(dscp);
    return band;
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
}

TrafficControlHelper
CbQueueDisc::GenerateDisc1FifoNPolicers(const string &queueSize, const TrafficClassifier& dscpsClassifier,
                                        double policingRate, double burstLength, const string& resultsPath) {
    system(("mkdir -p " + resultsPath).c_str());

    TrafficControlHelper policerTch;
    uint16_t handle = policerTch.SetRootQueueDisc(
            "ns3::CbQueueDisc",
            "MaxSize", StringValue(queueSize),
            "ChildQueuesClassifiers", TrafficClassifierValue(dscpsClassifier));

    policerTch.AddQueueDiscClasses (handle, dscpsClassifier.GetNumberOfClasses(), "ns3::QueueDiscClass");
    policerTch.AddChildQueueDisc (handle, dscpsClassifier.GetDscps2Band(0)->GetBand(), "ns3::FifoQueueDisc",
                                  "MaxSize", StringValue(queueSize));

    int mtu = 1500;
    int burst = max(int(policingRate * burstLength * 125000), 10 * mtu); // in byte
    for (uint32_t i = 1; i < dscpsClassifier.GetNumberOfClasses(); i++) {
        policerTch.AddChildQueueDisc(handle, dscpsClassifier.GetDscps2Band(i)->GetBand(), "ns3::TbfQueueDiscChild",
                                     "MaxSize", StringValue(to_string(mtu) + "B"),
                                     "Burst", UintegerValue(burst),
                                     "Mtu", UintegerValue (mtu),
                                     "Rate", DataRateValue(DataRate(to_string(policingRate) + "Mbps")),
                                     "PeakRate", DataRateValue(DataRate("0bps")),
                                     "QueueTraceOutput", StringValue(resultsPath + "/queue_events_" + to_string(i) + ".csv"));
    }

    return policerTch;
}
