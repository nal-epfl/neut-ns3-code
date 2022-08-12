//
// Created by nal on 01.11.21.
//

#ifndef WEHE_P_TOMOGRAPHY_INFINITETCPSENDER_H
#define WEHE_P_TOMOGRAPHY_INFINITETCPSENDER_H

#include <cstdlib>     //for using the function sleep

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "../../monitors_module/CwndMonitor.h"

using namespace ns3;
using namespace std;

class InfiniteTCPSender : public Application {

private:
    void StartApplication() override;
    void StopApplication() override;

    bool Send();
    void ScheduleNextSend();
    void ResumeApp(Ptr<Socket> localSocket, uint32_t txSpace);

    string _appTag;

    Ptr<Socket> _socket;
    Address _receiverAddress;
    string _tcpProtocol;

    uint32_t _pktSize; // size of sent packets
    DataRate _maxSendingRate;
    bool _enablePacing;

    uint32_t _nbSentPkts;
    bool _appPaused;
    EventId _sendEvent;

    bool _enableCwndMonitor = false;
    CwndMonitor* _cwndMonitor{};
    string _resultsFolder;


protected:
    void DoDispose() override;

public:
    static TypeId GetTypeId();

    InfiniteTCPSender();
    ~InfiniteTCPSender() override;
};




#endif //WEHE_P_TOMOGRAPHY_INFINITETCPSENDER_H
