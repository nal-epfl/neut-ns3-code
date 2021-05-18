//
// Created by nal on 07.09.20.
//

#ifndef WEHE_PLUS_TOMOGRAPHY_PPBBIDIRECTIONAL_H
#define WEHE_PLUS_TOMOGRAPHY_PPBBIDIRECTIONAL_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

#include "PPBSettings.h"

using namespace ns3;
using namespace std;

class PPBBidirectional {

private:
    Ptr<Node> _host1;
    Ptr<Node> _host2;

    uint32_t _pktSize;

    PPBSettings* _upPPBParams; // from host 1 to 2
    PPBSettings* _downPPBParams; // from host 2 to 1

public:
    PPBBidirectional();

    void SetHosts(const Ptr<Node> &host1, const Ptr<Node> &host2);
    void SetPktSize(uint32_t pktSize);
    void SetUpPPBParams(PPBSettings *UpPPBParams);
    void SetDownPPBParams(PPBSettings *downPPBParams);

    void Setup(ns3::Time startTime, ns3::Time endTime);

};


#endif //WEHE_PLUS_TOMOGRAPHY_PPBBIDIRECTIONAL_H
