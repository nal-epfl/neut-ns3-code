//
// Created by Zeinab Shmeis on 16.04.20.
//

#include "CbPolicingQueueDisc.h"


NS_LOG_COMPONENT_DEFINE("CbPolicingQueueDisc");

NS_OBJECT_ENSURE_REGISTERED(CbPolicingQueueDisc);

TokenBucket NullObject = 0;

TypeId CbPolicingQueueDisc::GetTypeId (void) {
    static TypeId tid = TypeId ("ns3::CbPolicingQueueDisc")
            .SetParent<QueueDisc> ()
            .SetGroupName ("TrafficControl")
            .AddConstructor<CbPolicingQueueDisc> ()
            .AddAttribute ("MaxSize",
                           "The max queue size",
                           QueueSizeValue (QueueSize ("1000p")),
                           MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                                  &QueueDisc::GetMaxSize),
                           MakeQueueSizeChecker ())
            .AddAttribute ("TokenBucket",
                           "A policer corresponding to specific class of traffic",
                           TokenBucketValue(NullObject),
                           MakeTokenBucketAccessor(&CbPolicingQueueDisc::AddTB),
                           MakeTokenBucketChecker())
            .AddAttribute ("TokenBucket1",
                           "A policer corresponding to specific class of traffic",
                           TokenBucketValue(NullObject),
                           MakeTokenBucketAccessor(&CbPolicingQueueDisc::AddTB),
                           MakeTokenBucketChecker())
    ;

    return tid;
}

CbPolicingQueueDisc::CbPolicingQueueDisc() : QueueDisc (QueueDiscSizePolicy::SINGLE_CHILD_QUEUE_DISC) {
    NS_LOG_FUNCTION (this);
}

CbPolicingQueueDisc::~CbPolicingQueueDisc() {
    NS_LOG_FUNCTION (this);
}

void CbPolicingQueueDisc::DoDispose(void) {
    NS_LOG_FUNCTION (this);
    QueueDisc::DoDispose ();
}

void CbPolicingQueueDisc::AddTB(TokenBucket tb) {
    if(tb == NullObject) return;
    NS_LOG_FUNCTION(this << tb);
    diff_services_TB[tb.GetTosId()] = tb;
}

bool CbPolicingQueueDisc::DoEnqueue(Ptr<QueueDiscItem> item) {
    NS_LOG_FUNCTION(this << item);

    bool retval = GetQueueDiscClass(0)->GetQueueDisc()->Enqueue(item);

    // If Queue::Enqueue fails, QueueDisc::Drop is called by the child queue
    // disc because QueueDisc::AddQueueDiscClass sets the drop callback
    NS_LOG_LOGIC("Current queue size: " << GetNPackets() << " packets, " << GetNBytes() << " bytes");

    return retval;
}

Ptr<QueueDiscItem> CbPolicingQueueDisc::DoDequeue() {
    NS_LOG_FUNCTION(this);
    Ptr<const QueueDiscItem> itemPeek = GetQueueDiscClass(0)->GetQueueDisc()->Peek();

    if (itemPeek) {

        // check if the packet has a policer corresponding to it
        Ptr<const Ipv4QueueDiscItem> ipItem = DynamicCast<const Ipv4QueueDiscItem>(itemPeek);
        Ipv4Header ipHeader = ipItem->GetHeader();
        uint8_t tos = ipHeader.GetTos();

        if(diff_services_TB.find(tos) == diff_services_TB.end()) { // Packet have no policer so immediately dequeue it
            NS_LOG_LOGIC ("Packet with tos: " << unsigned(tos) << " have no policer");

            Ptr<QueueDiscItem> item = GetQueueDiscClass(0)->GetQueueDisc()->Dequeue();

            if (!item) {
                NS_LOG_DEBUG ("That's odd! Expecting the peeked packet, we got no packet.");
                return item;
            }

            NS_LOG_LOGIC ("Current queue size: " << GetNPackets() << " packets, " << GetNBytes() << " bytes");
            return item;
        }
        else {
            NS_LOG_LOGIC ("Packet with a policer for tos " << unsigned(tos) << " is dequeued");

            uint32_t pktSize = itemPeek->GetSize ();
            NS_LOG_LOGIC ("Next packet size " << pktSize);

            if(diff_services_TB[tos].ConsumeTokens(pktSize)) {
                Ptr<QueueDiscItem> item = GetQueueDiscClass(0)->GetQueueDisc()->Dequeue();

                if (!item) {
                    NS_LOG_DEBUG ("That's odd! Expecting the peeked packet, we got no packet.");
                    return item;
                }

                NS_LOG_LOGIC ("Current queue size: " << GetNPackets () << " packets, " << GetNBytes () << " bytes");
                return item;
            }
            else {
                GetQueueDiscClass(0)->GetQueueDisc()->Dequeue();
                return 0;
            }
        }
    }
    return 0;
}


bool CbPolicingQueueDisc::CheckConfig() {
    NS_LOG_FUNCTION (this);
    if (GetNInternalQueues () > 0) {
        NS_LOG_ERROR ("CbPolicingQueueDisc cannot have internal queues");
        return false;
    }

    if (GetNPacketFilters () > 0) {
        NS_LOG_ERROR ("CbPolicingQueueDisc cannot have packet filters");
        return false;
    }

    if (GetNQueueDiscClasses () == 0) {
        // create a FIFO queue disc
        ObjectFactory factory;
        factory.SetTypeId ("ns3::FifoQueueDisc");
        Ptr<QueueDisc> qd = factory.Create<QueueDisc> ();

        if (!qd->SetMaxSize (GetMaxSize ())) {
            NS_LOG_ERROR ("Cannot set the max size of the child queue disc to that of CbPolicingQueueDisc");
            return false;
        }
        qd->Initialize ();

        Ptr<QueueDiscClass> c = CreateObject<QueueDiscClass> ();
        c->SetQueueDisc (qd);
        AddQueueDiscClass (c);
    }

    if (GetNQueueDiscClasses () != 1) {
        NS_LOG_ERROR ("CbPolicingQueueDisc needs 1 child queue disc");
        return false;
    }

    // Check each token bucket configuration
    for (std::pair<uint8_t, TokenBucket> element : diff_services_TB) {
        if (element.second.GetMtu() == 0) {
            Ptr<NetDeviceQueueInterface> ndqi = GetNetDeviceQueueInterface();
            Ptr<NetDevice> dev;
            // if the NetDeviceQueueInterface object is aggregated to a
            // NetDevice, get the MTU of such NetDevice
            if (ndqi && (dev = ndqi->GetObject<NetDevice>())) {
                element.second.SetMtu(dev->GetMtu());
            }
        }

        if (element.second.GetMtu() == 0 && element.second.GetPeakRate() > DataRate("0bps")) {
            NS_LOG_ERROR ("A non-null peak rate has been set, but the mtu is null. No packet will be dequeued");
            return false;
        }

        if (element.second.GetBurst() <= element.second.GetMtu()) {
            NS_LOG_WARN ("The size of the first bucket (" << element.second.GetBurst() << ") should be "
                               << "greater than the size of the second bucket (" << element.second.GetMtu() << ").");
        }

        if (element.second.GetPeakRate() > DataRate("0bps") && element.second.GetPeakRate() <= element.second.GetRate()) {
            NS_LOG_WARN ("The rate for the second bucket (" << element.second.GetPeakRate() << ") should be "
                               << "greater than the rate for the first bucket (" << element.second.GetRate() << ").");
        }
    }

    return true;
}

void CbPolicingQueueDisc::InitializeParams() {
    NS_LOG_FUNCTION (this);
    _id = EventId ();
}
