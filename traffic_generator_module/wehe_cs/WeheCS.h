//
// Created by nal on 07.02.21.
//

#ifndef WEHE_P_TOMOGRAPHY_WEHECS_H
#define WEHE_P_TOMOGRAPHY_WEHECS_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"

#include "../../helper_classes/HelperMethods.h"

#include "WeheClient.h"
#include "WeheServer.h"
#include "./wehe_tcp_cs/TCPWeheClient.h"
#include "./wehe_tcp_cs/TCPWeheServer.h"
#include "./wehe_udp_cs/UDPWeheClient.h"
#include "./wehe_udp_cs/UDPWeheServer.h"

using namespace ns3;
using namespace std;

class WeheCS {

private:

    static uint32_t APPS_COUNT;
    uint32_t _appId;
    uint16_t _serverPort;

    Ptr<Node> _clientNode, _serverNode;
    WeheServer* _serverApp;
    WeheClient* _clientApp;

    vector<WeheTraceItem> _traceItems;

    string _protocol;
    int _trafficTos = 0;

    bool _enableCwndMonitor = false;

    string _resultsFolder = "";

public:

    WeheCS(Ptr<Node> client, Ptr<Node> server, string protocol);

    uint16_t GetPort();
    void SetPort(uint16_t port);

    void SetTos(int tos);
    void SetResultsFolder(string resultsFolder);
    void EnableCwndMonitor();

    void LoadTrace(const string& traceFile);
    void StartApplication(ns3::Time startTime);
    void StopApplication(ns3::Time endTime);

    static WeheCS* CreateWeheCS(Ptr<Node> client, Ptr<Node> server, const string &trace, bool isTCP, uint8_t tos, const string &resultsPath);

};


#endif //WEHE_P_TOMOGRAPHY_WEHECS_H
