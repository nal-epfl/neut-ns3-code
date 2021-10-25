//
// Created by Zeinab Shmeis on 03.06.20.
//

#ifndef NEUTRALITY_POISSONUDPCLIENT_H
#define NEUTRALITY_POISSONUDPCLIENT_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "../../monitors_module/CwndMonitor.h"

using namespace ns3;
using namespace std;

class PoissonClient : public Application {

private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void SchedualeSend(void);
    bool Send(void);
    void ResumeApp(Ptr<Socket> localSocket, uint32_t txSpace);

    static uint32_t APPS_COUNT;
    uint32_t _appId;

    Ptr<RandomVariableStream> _interval; // interval between sending two packets
    uint32_t _size; // size of sent packets

    uint32_t _sent;
    Ptr<Socket> _socket;
    Address _peerAddress;
    uint16_t _peerPort;

    EventId _sendEvent;

    string _protocol;
    bool isTCP = false;

    bool _enableCwndMonitor;
    CwndMonitor* _cwndMonitor;
    string _resultsFolder = "";


protected:
    virtual void DoDispose(void);

public:
    static TypeId GetTypeId(void);

    PoissonClient();
    virtual ~PoissonClient();

    void SetRemote(Address ip, uint16_t port);
};


#endif //NEUTRALITY_POISSONUDPCLIENT_H
