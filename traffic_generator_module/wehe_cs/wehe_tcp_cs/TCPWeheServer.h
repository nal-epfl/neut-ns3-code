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

    bool Send(uint32_t payloadSize);
    void Recv(Ptr<Socket> socket);
    void ScheduleNextResponse();
    void ResumeResponse(Ptr<Socket> localSocket, uint32_t txSpace);

    uint32_t _appId;
    Ptr<Node> _server;

    InetSocketAddress _serverAddress;
    Ptr<Socket> _lSocket, _socket;

    vector<WeheTraceItem> _traceItems;
    uint32_t _traceItemIdx, _nbBytesRx;
    void CheckForNextResponse(uint32_t nbBytesRx);

    ns3::Time _startTime;

    bool _enableCwndMonitor = false;
    CwndMonitor* _cwndMonitor;
    vector<RxEvent> _rxEvents;
    string _resultsFolder = "";

    // This part is to control how much data is sent to the socket
    bool sendingResponse = false;

public:

    TCPWeheServer(uint32_t appId, Ptr<Node> server, InetSocketAddress serverAddress);

    void LoadTrace(vector<WeheTraceItem> &traceItems);
    void SetResultsFolder(string resultsFolder);
    void EnableCwndMonitor();

    void StartApplication();
    void StopApplication();

};


#endif //WEHE_P_TOMOGRAPHY_TCPWEHESERVER_H
