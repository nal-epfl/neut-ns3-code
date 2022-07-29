//
// Created by Zeinab Shmeis on 16.04.20.
//
// This is an implementation of a class based TBF

#ifndef NEUTRALITY_CBPOLICINGQUEUEDISC_H
#define NEUTRALITY_CBPOLICINGQUEUEDISC_H

#include <utility>

#include "ns3/core-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/internet-module.h"
#include "ns3/net-device-queue-interface.h"

#include "../helper_classes/HelperMethods.h"
#include "Dscps2QueueBand.h"

using namespace ns3;
using namespace std;
using namespace helper_methods;

class TrafficClassifier {
private:
    vector<Dscps2QueueBand*> _dscps2bands;

public:
    TrafficClassifier(): _dscps2bands({}) {};
    explicit TrafficClassifier(vector<Dscps2QueueBand*> dscps2bands): _dscps2bands(std::move(dscps2bands)) {};

    [[nodiscard]] vector<Dscps2QueueBand*> GetDscps2Bands() const { return _dscps2bands; };
    void push_back(Dscps2QueueBand* dscps2band) { _dscps2bands.push_back(dscps2band); };
    [[nodiscard]] uint32_t GetNumberOfClasses() const { return _dscps2bands.size(); };
    [[nodiscard]] Dscps2QueueBand* GetDscps2Band(uint32_t idx) const { return _dscps2bands[idx]; }

    uint16_t ClassifyDscp(uint8_t dscp);

    friend ostream &operator<<(ostream &os, const TrafficClassifier &classifier);
    friend istream &operator>>(istream &is, TrafficClassifier &classifier);
};

class CbQueueDisc : public QueueDisc {

    private:
        TrafficClassifier _classifier;
        uint16_t _nbBands = 0, _nextBand;
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

        void SetTrafficClassifier(TrafficClassifier classifier);

        static TrafficControlHelper
        GenerateDisc1FifoNPolicers(const string &queueSize, const TrafficClassifier& dscpsClassifier,
                                   double policingRate, double burstLength, const string& resultsPath);

};

ATTRIBUTE_HELPER_HEADER(TrafficClassifier);

#endif //NEUTRALITY_CBPOLICINGQUEUEDISC_H
