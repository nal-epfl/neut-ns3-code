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

typedef vector<uint8_t> DscpMap;

class CbQueueDisc : public QueueDisc {

    private:
        map<uint8_t, uint16_t> _dscp2band;
        uint16_t _nbBands{}, _nextBand;
        EventId _id;

        bool DoEnqueue (Ptr<QueueDiscItem> item) override;
        Ptr<QueueDiscItem> DoDequeue () override;
        Ptr<const QueueDiscItem> DoPeek() override;
        bool CheckConfig () override;
        void InitializeParams () override;
        uint16_t Classify (const Ptr<QueueDiscItem>& item);


    protected:
        void DoDispose () override;

    public:
        static TypeId GetTypeId ();
        CbQueueDisc();
        ~CbQueueDisc() override;

        void SetDscpMap (DscpMap dscpMap);

        static TrafficControlHelper
        GenerateDisc1FifoNPolicers(const string &queueSize, const vector<uint8_t> &childDiscsDscp,
                                   double policingRate, double burstLength, const string& resultsPath);

};

std::ostream &operator << (std::ostream &os, const DscpMap &dscpMap);

std::istream &operator >> (std::istream &is, DscpMap &dscpMap);

ATTRIBUTE_HELPER_HEADER(DscpMap);

#endif //NEUTRALITY_CBPOLICINGQUEUEDISC_H
