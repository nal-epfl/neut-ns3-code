//
// Created by nal on 08.02.21.
//

#ifndef WEHE_P_TOMOGRAPHY_BACKGROUNDREPLAY_H
#define WEHE_P_TOMOGRAPHY_BACKGROUNDREPLAY_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

#include "../../helper_classes/HelperMethods.h"

using namespace ns3;
using namespace std;
using namespace helper_methods;

class BackgroundReplay {

private:
    static uint32_t SOCKET_COUNT;

    Ptr<Node> _sender;
    Ptr<Node> _receiver;

    void RunSingleTrace(const string& tracePath, const string& protocol, uint8_t dscp);

public:
    BackgroundReplay(const Ptr<Node>& sender, const Ptr<Node>& receiver);

    void RunAllTraces(const string& tracesPath, uint32_t nbTCPFlows, uint32_t nbUDPFlows, uint8_t dscp=0);
    void RunAllTraces(const string& tracesPath, uint8_t dscp=0);
    void RunSpecificTraces(const vector<string>& tcpTracesPath, const vector<string>& udpTracesPath, uint8_t dscp);
    void RunTracesWithRandomThrottledTCPFlows(const string& tracesPath, double throttledProb, uint8_t thottledDscp);
    void RunTracesWithRandomThrottledUDPFlows(const string& tracesPath, double throttledProb, uint8_t thottledDscp);
};


#endif //WEHE_P_TOMOGRAPHY_BACKGROUNDREPLAY_H
