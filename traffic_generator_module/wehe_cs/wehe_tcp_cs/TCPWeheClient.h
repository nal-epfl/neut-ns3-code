//
// Created by nal on 25.04.21.
//

#ifndef WEHE_P_TOMOGRAPHY_TCPWEHECLIENT_H
#define WEHE_P_TOMOGRAPHY_TCPWEHECLIENT_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "../../../monitors_module/CwndMonitor.h"
#include "../WeheCS_Utility.h"
#include "../WeheClient.h"

using namespace ns3;
using namespace std;

class TCPWeheClient : public WeheClient {

private:

    void Send(uint32_t payloadSize);
    void Recv(Ptr<Socket> socket);
    void ScheduleNextSendingEvents();

    uint32_t _appId;
    Ptr<Node> _client;

    InetSocketAddress _serverAddress;
    Ptr<Socket> _socket;

    vector<WeheTraceItem> _traceItems;
    uint32_t _traceItemIdx, _nbBytesRx;
    void DecideOnNextSend(uint32_t nbBytesRx);

    ns3::Time _startTime;

    bool _enableCwndMonitor = false;
    CwndMonitor* _cwndMonitor;
    vector<RxEvent> _rxEvents;
    string _resultsFolder = "";

public:

    TCPWeheClient(uint32_t appId, Ptr<Node> &client, InetSocketAddress &serverAddress);

    void LoadTrace(vector<WeheTraceItem> &traceItems);
    void SetResultsFolder(string resultsFolder);
    void EnableCwndMonitor();

    void StartApplication();
    void StopApplication();

};


#endif //WEHE_P_TOMOGRAPHY_TCPWEHECLIENT_H
