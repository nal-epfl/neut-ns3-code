//
// Created by nal on 08.02.21.
//

#ifndef WEHE_P_TOMOGRAPHY_MULTIPLEREPLAYCLIENTS_H
#define WEHE_P_TOMOGRAPHY_MULTIPLEREPLAYCLIENTS_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

using namespace ns3;
using namespace std;


class MultipleReplayClients {

private:
    static uint32_t SOCKET_COUNT;

    Ptr<Node> _sender;
    Ptr<Node> _receiver;

    void RunSingleTrace(const string& tracePath, const string& protocol, uint8_t tos);

public:
    MultipleReplayClients(const Ptr<Node>& sender, const Ptr<Node>& receiver);

    void RunAllTraces(const string& tracesPath, uint32_t nbTCPFlows, uint32_t nbUDPFlows, uint8_t tos=0);
    void RunAllTraces(const string& tracesPath, uint8_t tos=0);
    void RunSpecificTraces(const vector<string>& tcpTracesPath, const vector<string>& udpTracesPath, uint8_t tos);
    void RunTracesWithRandomThrottledTCPFlows(const string& tracesPath, double throttledProb, uint8_t thottledTos);
    void RunTracesWithRandomThrottledUDPFlows(const string& tracesPath, double throttledProb, uint8_t thottledTos);
};


#endif //WEHE_P_TOMOGRAPHY_MULTIPLEREPLAYCLIENTS_H
