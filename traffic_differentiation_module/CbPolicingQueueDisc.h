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

#include "TokenBucket.h"

using namespace ns3;
using namespace std;

class CbPolicingQueueDisc : public QueueDisc {

    private:
        map<uint8_t, TokenBucket> diff_services_TB;
        EventId _id;

        virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
        virtual Ptr<QueueDiscItem> DoDequeue (void);
        virtual bool CheckConfig (void);
        virtual void InitializeParams (void);

    protected:
        virtual void DoDispose (void);

    public:
        static TypeId GetTypeId (void);
        CbPolicingQueueDisc();
        virtual ~CbPolicingQueueDisc();

        void AddTB (TokenBucket tb);

};


#endif //NEUTRALITY_CBPOLICINGQUEUEDISC_H
