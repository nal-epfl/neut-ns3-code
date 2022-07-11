//
// Created by Zeinab Shmeis on 03.06.20.
//

#ifndef NEUTRALITY_PROBESSENDER_H
#define NEUTRALITY_PROBESSENDER_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "../../monitors_module/CwndMonitor.h"

using namespace ns3;
using namespace std;

class ProbesSender : public Application {

private:
    void StartApplication() override;
    void StopApplication() override;

    bool Send();
    void ScheduleNextSend();
    void ResumeApp(Ptr<Socket> localSocket, uint32_t txSpace);

    string _appTag;

    Ptr<Socket> _socket;
    Address _receiverAddress;
    string _protocol;

    uint32_t _pktSize; // size of sent packets
    Ptr<RandomVariableStream> _interval; // interval between sending two packets

    uint32_t _nbSentPkts;
    EventId _sendEvent;
    bool _appPaused;

    bool _enableCwndMonitor = false;
    CwndMonitor* _cwndMonitor{};
    string _resultsFolder;

protected:
    void DoDispose() override;

public:
    static TypeId GetTypeId();

    ProbesSender();
    ~ProbesSender() override;
};


#endif //NEUTRALITY_PROBESSENDER_H
