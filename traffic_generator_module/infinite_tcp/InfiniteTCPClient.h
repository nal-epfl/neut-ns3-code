//
// Created by nal on 01.11.21.
//

#ifndef WEHE_P_TOMOGRAPHY_INFINITETCPCLIENT_H
#define WEHE_P_TOMOGRAPHY_INFINITETCPCLIENT_H

#include <cstdlib>     //for using the function sleep

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "../../monitors_module/CwndMonitor.h"

using namespace ns3;
using namespace std;

class InfiniteTCPClient : public Application {

private:
    void StartApplication() override;
    void StopApplication() override;

    void ScheduleSend();
    bool Send();
    void ResumeApp(Ptr<Socket> localSocket, uint32_t txSpace);

    static uint32_t APPS_COUNT;
    uint32_t _appId;

    uint32_t _pktSize, _nbSentPkts; // size of sent packets
    DataRate _maxSendingRate;

    Ptr<Socket> _socket;
    string _tcpProtocol;
    Address _address;
    uint16_t _port;

    bool _appPaused;
    EventId _sendEvent;

    bool _enableCwndMonitor = false;
    CwndMonitor* _cwndMonitor{};
    string _resultsFolder;


protected:
    void DoDispose() override;

public:
    static TypeId GetTypeId();

    InfiniteTCPClient();
    ~InfiniteTCPClient() override;
};




#endif //WEHE_P_TOMOGRAPHY_INFINITETCPCLIENT_H
