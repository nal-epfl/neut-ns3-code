//
// Created by nal on 01.11.21.
//

#ifndef WEHE_P_TOMOGRAPHY_INFINITETCPCLIENT_H
#define WEHE_P_TOMOGRAPHY_INFINITETCPCLIENT_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "../../monitors_module/CwndMonitor.h"

using namespace ns3;
using namespace std;

class InfiniteTCPClient : public Application {

private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void SchedualeSend(void);
    bool Send(void);
    void ResumeApp(Ptr<Socket> localSocket, uint32_t txSpace);

    static uint32_t APPS_COUNT;
    uint32_t _appId;

    uint32_t _pktSize, _nbSentPkts; // size of sent packets

    Ptr<Socket> _socket;
    Address _peerAddress;
    uint16_t _peerPort;

    bool _appPaused;
    EventId _sendEvent;

    bool _enableCwndMonitor;
    CwndMonitor* _cwndMonitor;
    string _resultsFolder = "";


protected:
    virtual void DoDispose(void);

public:
    static TypeId GetTypeId(void);

    InfiniteTCPClient();
    virtual ~InfiniteTCPClient();

    void SetRemote(Address ip, uint16_t port);
};




#endif //WEHE_P_TOMOGRAPHY_INFINITETCPCLIENT_H
