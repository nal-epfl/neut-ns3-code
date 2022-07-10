//
// Created by nal on 25.04.21.
//

#ifndef WEHE_P_TOMOGRAPHY_TCPWEHESERVER_H
#define WEHE_P_TOMOGRAPHY_TCPWEHESERVER_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "../../../monitors_module/CwndMonitor.h"
#include "../WeheCS_Utility.h"
#include "../WeheServer.h"


using namespace ns3;
using namespace std;

class TCPWeheServer : public WeheServer {

private:

    void SetupConnection(Ptr<Socket> socket);
    void HandleTCPAccept (Ptr<Socket> socket, const Address& from);

    bool Send(const WeheTraceItem& item);
    void Recv(Ptr<Socket> socket);
    void ScheduleNextResponse();
    void ResumeResponse(Ptr<Socket> localSocket, uint32_t txSpace);

    string _appTag;
    Ptr<Node> _server;

    InetSocketAddress _serverAddress;
    Ptr<Socket> _lSocket, _socket;

    vector<WeheTraceItem> _traceItems;
    uint32_t _traceItemIdx = 0, _nbBytesRx = 0;
    void CheckForNextResponse(uint32_t nbBytesRx);

    ns3::Time _startTime;

    bool _enableCwndMonitor = false;
    CwndMonitor* _cwndMonitor{};
    vector<RxEvent> _rxEvents;
    string _resultsFolder;

    // This part is to control how much data is sent to the socket
    bool sendingResponse = false;

    // This to handle proper application stop
    EventId _sendEvent;
    bool appStopped = false;

public:

    TCPWeheServer(string appTag, const Ptr<Node>& server, InetSocketAddress serverAddress);

    void LoadTrace(vector<WeheTraceItem> &traceItems) override;
    void SetResultsFolder(string resultsFolder) override;
    void SetDscp(int tos) override;
    void EnableCwndMonitor() override;

    void StartApplication() override;
    void StopApplication() override;

};


#endif //WEHE_P_TOMOGRAPHY_TCPWEHESERVER_H
