//
// Created by Zeinab Shmeis on 25.03.20.
//

#ifndef DUMBBELL_TOPOLOGY_CWNDMONITOR_H
#define DUMBBELL_TOPOLOGY_CWNDMONITOR_H


#include <cstdint>

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

using namespace ns3;
using namespace std;

template<typename T>

struct ChangeTuple {
    T oldVal;
    T newVal;
    ns3::Time time;

    [[nodiscard]] string to_string() const;
};


class CwndMonitor {

    private:
        uint32_t _nodeId;
        uint32_t _cwndId;
        Ptr<Socket> _socket = 0;

        ns3::Time _startTime;
        string _outputFolderPath;

        vector<ChangeTuple<uint32_t>> cwndChanges;
        vector<ChangeTuple<DataRate>> pacingRateChanges;
        vector<ChangeTuple<TcpSocketState::TcpCongState_t>> congStateChanges;
        vector<ChangeTuple<ns3::Time>> rtoChanges;
        vector<ChangeTuple<ns3::Time>> rttChanges;

        void ConnectTraceSource();
        void RecordCwndChange(string context, uint32_t oldval, uint32_t newval);
        void RecordPacingRate(string context, DataRate oldval, DataRate newval);
        void RecordCongStateChange(string context, TcpSocketState::TcpCongState_t oldval, TcpSocketState::TcpCongState_t newval);
        void RecordRTO(string context, ns3::Time t1, ns3::Time t2);
        void RecordRTT(string context, ns3::Time t1, ns3::Time t2);

        string GetDisplayTime(const ns3::Time& time);

    public:
        CwndMonitor(uint32_t nodeId, uint32_t cwndId, const ns3::Time& startTime, string outputFolderPath);
        CwndMonitor(Ptr<Socket> socket, string outputFolderPath);
        void SaveCwndChanges();
        void SavePacingRateChanges();
        void SaveCongStateChanges();
        void SaveRtoChanges();
        void SaveRttChanges();

};

#endif //DUMBBELL_TOPOLOGY_CWNDMONITOR_H
