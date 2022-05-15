//
// Created by nal on 25.04.21.
//

#ifndef WEHE_P_TOMOGRAPHY_UDPWEHESERVER_H
#define WEHE_P_TOMOGRAPHY_UDPWEHESERVER_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "../../../monitors_module/CwndMonitor.h"
#include "../WeheCS_Utility.h"
#include "../WeheServer.h"


using namespace ns3;
using namespace std;


class UDPWeheServer : public WeheServer {

private:

    void SetupConnection(Ptr<Socket> socket);
    void HandleUDPAccept (Ptr<Socket> socket);

    void Send(uint32_t payloadSize, uint32_t seqNb);
    void Recv(Ptr<Socket> socket);
    void ScheduleNextSendingEvents();

    uint32_t _appId;
    Ptr<Node> _server;

    InetSocketAddress _serverAddress;
    Ptr<Socket> _lSocket, _socket;

    vector<WeheTraceItem> _traceItems;
    uint32_t _traceItemIdx;

    ns3::Time _startTime;

    vector<RxEvent> _rxEvents;
    string _resultsFolder = "";

    int _trafficTos = 0;

public:

    UDPWeheServer(uint32_t appId, Ptr<Node> server, InetSocketAddress serverAddress);

    void LoadTrace(vector<WeheTraceItem> &traceItems);
    void SetResultsFolder(string resultsFolder);
    void SetTos(int tos);

    void StartApplication();
    void StopApplication();

    void EnableCwndMonitor();
};


#endif //WEHE_P_TOMOGRAPHY_UDPWEHESERVER_H
