//
// Created by nal on 25.04.21.
//

#ifndef WEHE_P_TOMOGRAPHY_UDPWEHECLIENT_H
#define WEHE_P_TOMOGRAPHY_UDPWEHECLIENT_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "../../../monitors_module/CwndMonitor.h"
#include "../WeheCS_Utility.h"
#include "../WeheClient.h"

using namespace ns3;
using namespace std;

class UDPWeheClient : public WeheClient {

private:

    void Send(uint32_t payloadSize, uint32_t seqNb);
    void Recv(Ptr<Socket> socket);
    void ScheduleNextSendingEvents();

    uint32_t _appId;
    Ptr<Node> _client;

    InetSocketAddress _serverAddress;
    Ptr<Socket> _socket;

    vector<WeheTraceItem> _traceItems;
    uint32_t _traceItemIdx;

    ns3::Time _startTime;

    vector<RxEvent> _rxEvents;
    string _resultsFolder = "";

public:

    UDPWeheClient(uint32_t appId, Ptr<Node> &client, InetSocketAddress &serverAddress);

    void LoadTrace(vector<WeheTraceItem> &traceItems);
    void SetResultsFolder(string resultsFolder);
    void EnableCwndMonitor();

    void StartApplication();
    void StopApplication();

};


#endif //WEHE_P_TOMOGRAPHY_UDPWEHECLIENT_H
