//
// Created by Zeinab Shmeis on 16.04.20.
//
// This is an implementation of a class based TBF

#ifndef NEUTRALITY_CBPOLICINGQUEUEDISC_H
#define NEUTRALITY_CBPOLICINGQUEUEDISC_H

#include "ns3/core-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/internet-module.h"
#include "ns3/net-device-queue-interface.h"

using namespace ns3;
using namespace std;

typedef vector<uint8_t> TosMap;

class CbQueueDisc : public QueueDisc {

    private:
        map<uint8_t, uint16_t> _tos2band;
        uint16_t _nbBands, _nextBand;
        EventId _id;

        virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
        virtual Ptr<QueueDiscItem> DoDequeue (void);
        virtual Ptr<const QueueDiscItem> DoPeek(void);
        virtual bool CheckConfig (void);
        virtual void InitializeParams (void);
        uint16_t Classify (Ptr<QueueDiscItem> item);


    protected:
        virtual void DoDispose (void);

    public:
        static TypeId GetTypeId (void);
        CbQueueDisc();
        virtual ~CbQueueDisc();

        void SetTosMap (TosMap tosMap);

};

std::ostream &operator << (std::ostream &os, const TosMap &tosMap);

std::istream &operator >> (std::istream &is, TosMap &tosMap);

ATTRIBUTE_HELPER_HEADER(TosMap);

#endif //NEUTRALITY_CBPOLICINGQUEUEDISC_H
