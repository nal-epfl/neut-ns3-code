//
// Created by nal on 25.10.21.
//

#ifndef WEHE_P_TOMOGRAPHY_NEW_MEASURREPLAYCLIENT_H
#define WEHE_P_TOMOGRAPHY_NEW_MEASURREPLAYCLIENT_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "../../monitors_module/CwndMonitor.h"

using namespace ns3;
using namespace std;

struct TraceReplayItem {
    uint32_t frameNb;
    Time timestamp;
    uint32_t payloadSize;

    string ToString() {
        return "Wehe trace item: frameNb=" + to_string(frameNb) + ", timestamp=" + to_string(timestamp.GetSeconds())
               + ", payloadSize=" + to_string(payloadSize);
    }
};


class MeasurReplayClient : public Application {

private:
    static int CLIENTS_COUNT;

    virtual void StartApplication(void);

    virtual void StopApplication(void);
    bool Send(uint32_t payload_size);

    void LoadTrace();
    void ScheduleNextSend();
    void ResumeApp(Ptr<Socket> localSocket, uint32_t txSpace);

    int _clientId;
    Ptr<Socket> _socket;
    Address _peerAddress;
    uint16_t _peerPort;
    string _protocol;
    string _traceFilename;

    bool _enableCwndMonitor;
    CwndMonitor* _cwndMonitor;
    string _congAlgoFolder;

    vector<TraceReplayItem> _traceItems;
    uint32_t _traceItemIdx;
    ns3::Time _startTime;
    bool _appPaused;
    EventId _sendEvent;

protected:
    virtual void DoDispose(void);

public:
    static TypeId GetTypeId(void);

    MeasurReplayClient();
    virtual ~MeasurReplayClient();

    void SetRemote(Address ip, uint16_t port);
};


#endif //WEHE_P_TOMOGRAPHY_NEW_MEASURREPLAYCLIENT_H
