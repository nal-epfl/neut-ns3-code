//
// Created by nal on 25.10.21.
//

#ifndef WEHE_P_TOMOGRAPHY_NEW_MEASUREMENTREPLAYSENDER_H
#define WEHE_P_TOMOGRAPHY_NEW_MEASUREMENTREPLAYSENDER_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "../../monitors_module/CwndMonitor.h"
#include "../../helper_classes/HelperMethods.h"

using namespace ns3;
using namespace std;
using namespace helper_methods;

struct TraceReplayItem {
    uint32_t frameNb;
    Time timestamp;
    uint32_t payloadSize;

    [[nodiscard]] string ToString() const {
        return "Trace replay item: frameNb=" + to_string(frameNb) + ", timestamp=" + to_string(timestamp.GetSeconds())
               + ", payloadSize=" + to_string(payloadSize);
    }
};

class MeasurementReplaySender : public Application {

private:
    void LoadTrace();
    void StartApplication() override;
    void StopApplication() override;

    bool Send(const TraceReplayItem &item);
    void ScheduleNextSend();
    void ResumeApp(Ptr<Socket> localSocket, uint32_t txSpace);

    string _appTag;

    Ptr<Socket> _socket;
    Address _receiverAddress;
    string _protocol;

    string _traceFilename;
    vector<TraceReplayItem> _traceItems;
    uint32_t _traceItemIdx;

    bool _enablePacing;

    ns3::Time _startTime;
    bool _appPaused;
    EventId _sendEvent;

    bool _enableCwndMonitor = false;
    CwndMonitor* _cwndMonitor{};
    string _resultsFolder;

protected:
    void DoDispose() override;

public:
    static TypeId GetTypeId();

    MeasurementReplaySender();
    ~MeasurementReplaySender() override;

};


#endif //WEHE_P_TOMOGRAPHY_NEW_MEASUREMENTREPLAYSENDER_H
