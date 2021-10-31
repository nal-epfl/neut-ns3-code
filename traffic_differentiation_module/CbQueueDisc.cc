//
// Created by Zeinab Shmeis on 16.04.20.
//

#include "CbQueueDisc.h"


NS_LOG_COMPONENT_DEFINE("CbQueueDisc");

NS_OBJECT_ENSURE_REGISTERED(CbQueueDisc);

ATTRIBUTE_HELPER_CPP (TosMap);

std::ostream &operator << (std::ostream &os, const TosMap &tosMap) {
    std::copy (tosMap.begin (), tosMap.end () - 1, std::ostream_iterator<uint8_t>(os, " "));
    os << tosMap.back ();
    return os;
}

std::istream &operator >> (std::istream &is, TosMap &tosMap) {
    uint8_t val;
    while (is >> val) {
        tosMap.push_back(val);
    }
    return is;
}

TypeId CbQueueDisc::GetTypeId (void) {
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
            .AddAttribute ("TosMap",
                           "The different type service to classify based on",
                           TosMapValue(TosMap{0}),
                           MakeTosMapAccessor(&CbQueueDisc::SetTosMap),
                           MakeTosMapChecker())
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

void CbQueueDisc::DoDispose(void) {
    NS_LOG_FUNCTION (this);
    QueueDisc::DoDispose ();
}

void CbQueueDisc::SetTosMap(TosMap tosMap) {
    NS_LOG_FUNCTION(this << tosMap);
    for(int i = 0; i < tosMap.size(); i++) {
        _tos2band[tosMap[i]] = i;
    }
    _nbBands = tosMap.size();
}

uint16_t CbQueueDisc::Classify(Ptr<QueueDiscItem> item) {
    // check what type of service corresponding to this packet
    Ptr<const Ipv4QueueDiscItem> ipItem = DynamicCast<const Ipv4QueueDiscItem>(item);
    Ipv4Header ipHeader = ipItem->GetHeader();
    uint8_t tos = ipHeader.GetTos();

    if(_tos2band.find(tos) == _tos2band.end()) { // this tos is not defined in the map
        return 0;
    }

    return _tos2band[tos];
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

    Ptr<QueueDiscItem> item = 0;

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

Ptr<const QueueDiscItem> CbQueueDisc::DoPeek(void) {
    NS_LOG_FUNCTION(this);

    Ptr<const QueueDiscItem> item = 0;

    for (uint32_t i = 0; i < _nbBands; i++) {
        uint32_t nextBand = (_nextBand + i) % _nbBands;
        if ((item = GetQueueDiscClass (nextBand)->GetQueueDisc ()->Peek ()) != 0)
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
