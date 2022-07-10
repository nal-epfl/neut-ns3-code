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

    string _appTag;
    Ptr<Node> _server;

    InetSocketAddress _serverAddress;
    Ptr<Socket> _lSocket, _socket;

    vector<WeheTraceItem> _traceItems;
    uint32_t _traceItemIdx;

    ns3::Time _startTime;

    vector<RxEvent> _rxEvents;
    string _resultsFolder;

    int _trafficTos = 0;

public:

    UDPWeheServer(string appTag, const Ptr<Node>& server, InetSocketAddress serverAddress);

    void LoadTrace(vector<WeheTraceItem> &traceItems) override;
    void SetResultsFolder(string resultsFolder) override;
    void SetDscp(int dscp) override;

    void StartApplication() override;
    void StopApplication() override;

    void EnableCwndMonitor() override;
};


#endif //WEHE_P_TOMOGRAPHY_UDPWEHESERVER_H
