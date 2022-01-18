//
// Created by nal on 31.08.20.
//

#ifndef WEHE_PLUS_TOMOGRAPHY_TRACEREPLAYCLIENT_H
#define WEHE_PLUS_TOMOGRAPHY_TRACEREPLAYCLIENT_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "../../monitors_module/CwndMonitor.h"

using namespace ns3;
using namespace std;

class TraceReplayClient : public Application {

private:
    static int CLIENTS_COUNT;

    virtual void StartApplication(void);
    void PrepareSocket();

    virtual void StopApplication(void);
    void Send(uint32_t payload_size);

    void ScheduleSendEvents();

    int _clientId;
    Ptr<Socket> _socket;
    Address _peerAddress;
    uint16_t _peerPort;
    string _protocol;
    string _traceFilename;

    bool _enableCwndMonitor;
    CwndMonitor* _cwndMonitor;
    string _congAlgoFolder;

    uint32_t _nbSentPkts;
    EventId _sendEvent;

protected:
    virtual void DoDispose(void);

public:
    static TypeId GetTypeId(void);

    TraceReplayClient();
    virtual ~TraceReplayClient();

    void SetRemote(Address ip, uint16_t port);

};


#endif //WEHE_PLUS_TOMOGRAPHY_TRACEREPLAYCLIENT_H
