//
// Created by Zeinab Shmeis on 25.03.20.
//

#include "CwndMonitor.h"

#include <utility>

template<typename T>
string ChangeTuple<T>::to_string() const {
    std::ostringstream strout;
    strout << oldVal << "\t" << newVal << "\t" << time;
    return strout.str();
}

CwndMonitor::CwndMonitor(uint32_t nodeId, uint32_t cwndId, const ns3::Time& startTime, string outputFolderPath) {
    _nodeId = nodeId;
    _cwndId = cwndId;
    _startTime = startTime;
    _outputFolderPath = std::move(outputFolderPath);
    system(("mkdir -p " + _outputFolderPath).c_str());

    Simulator::Schedule(_startTime, &CwndMonitor::ConnectTraceSource, this);
}

CwndMonitor::CwndMonitor(Ptr<Socket> socket, string outputFolderPath) {
    _socket = socket;
    _startTime = ns3::Now();
    _outputFolderPath = std::move(outputFolderPath);
    system(("mkdir -p " + _outputFolderPath).c_str());

    Simulator::Schedule(Seconds(0), &CwndMonitor::ConnectTraceSource, this);
}

void CwndMonitor::ConnectTraceSource() {
    if (_socket != 0) {
        _socket->TraceConnect("CongestionWindow", "", MakeCallback(&CwndMonitor::RecordCwndChange, this));
        _socket->TraceConnect("CongestionWindowInflated", "", MakeCallback(&CwndMonitor::RecordCwndInfChange, this));
        _socket->TraceConnect("PacingRate", "", MakeCallback(&CwndMonitor::RecordPacingRate, this));
        _socket->TraceConnect("CongState", "", MakeCallback(&CwndMonitor::RecordCongStateChange, this));
        _socket->TraceConnect("RTO", "", MakeCallback(&CwndMonitor::RecordRTO, this));
        _socket->TraceConnect("RTT", "", MakeCallback(&CwndMonitor::RecordRTT, this));
    }
    else {
        string socketPath = "/NodeList/" + to_string(_nodeId) + "/$ns3::TcpL4Protocol/SocketList/" + to_string(_cwndId);
        Config::Connect(socketPath + "/CongestionWindow", MakeCallback(&CwndMonitor::RecordCwndChange, this));
        Config::Connect(socketPath + "/CongState", MakeCallback(&CwndMonitor::RecordCongStateChange, this));
        Config::Connect(socketPath + "/RTO", MakeCallback(&CwndMonitor::RecordRTO, this));
        Config::Connect(socketPath + "/RTT", MakeCallback(&CwndMonitor::RecordRTT, this));
    }
}

void CwndMonitor::RecordCwndChange(string context, uint32_t oldval, uint32_t newval) {
    cwndChanges.push_back({oldval, newval, Simulator::Now()});
}

void CwndMonitor::RecordCwndInfChange(string context, uint32_t oldval, uint32_t newval) {
    cwndChanges.push_back({oldval, newval, Simulator::Now()});
}

void CwndMonitor::RecordPacingRate(string context, DataRate oldval, DataRate newval) {
    cout << "pacing rate: " << newval << endl;
    pacingRateChanges.push_back({oldval, newval, Simulator::Now()});
}

void CwndMonitor::RecordCongStateChange(string context, TcpSocketState::TcpCongState_t oldval, TcpSocketState::TcpCongState_t newval) {
    congStateChanges.push_back({oldval, newval, Simulator::Now()});
}

void CwndMonitor::RecordRTO(string context, ns3::Time t1, ns3::Time t2) {
    rtoChanges.push_back({t1, t2, Simulator::Now()});
}

void CwndMonitor::RecordRTT(string context, ns3::Time t1, ns3::Time t2) {
    rttChanges.push_back({t1, t2, Simulator::Now()});
}

string CwndMonitor::GetDisplayTime(const ns3::Time& time) {
    return to_string((time - _startTime).GetNanoSeconds() / 1e9);
}

void CwndMonitor::SaveCwndChanges() {
    ofstream outfile;
    outfile.open(_outputFolderPath + "/cwnd_changes.csv");
    for(auto cwndChange : cwndChanges) {
        outfile << cwndChange.oldVal << "," << cwndChange.newVal << "," << GetDisplayTime(cwndChange.time) << endl;
    }
    outfile.close();
}

void CwndMonitor::SaveCwndInfChanges() {
    ofstream outfile;
    outfile.open(_outputFolderPath + "/cwnd_inflated_changes.csv");
    for(auto cwndChange : cwndInfChanges) {
        outfile << cwndChange.oldVal << "," << cwndChange.newVal << "," << GetDisplayTime(cwndChange.time) << endl;
    }
    outfile.close();
}

void CwndMonitor::SavePacingRateChanges() {
    ofstream outfile;
    outfile.open(_outputFolderPath + "/pacing_rate_changes.csv");
    for(auto pacingRate : pacingRateChanges) {
        outfile << pacingRate.oldVal << "," << pacingRate.newVal << "," << GetDisplayTime(pacingRate.time) << endl;
    }
    outfile.close();
}

void CwndMonitor::SaveCongStateChanges() {
    ofstream outfile;
    outfile.open(_outputFolderPath + "/cong_state_changes.csv");
    for(auto congStateChange : congStateChanges) {
        outfile << TcpSocketState::TcpCongStateName[congStateChange.oldVal] << "," <<
                TcpSocketState::TcpCongStateName[congStateChange.newVal] << "," <<
                GetDisplayTime(congStateChange.time) << endl;
    }
    outfile.close();
}

void CwndMonitor::SaveRtoChanges() {
    ofstream outfile;
    outfile.open(_outputFolderPath + "/rto_changes.csv");
    for(auto rtoChange : rtoChanges) {
        outfile << to_string(rtoChange.oldVal.GetNanoSeconds()/1e9) << "," <<
                to_string(rtoChange.newVal.GetNanoSeconds()/1e9) << "," <<
                GetDisplayTime(rtoChange.time) << endl;
    }
    outfile.close();
}

void CwndMonitor::SaveRttChanges() {
    ofstream outfile;
    outfile.open(_outputFolderPath + "/rtt_changes.csv");
    for(auto rttChange : rttChanges) {
        outfile << to_string(rttChange.oldVal.GetNanoSeconds() / 1e9) << "," <<
                to_string(rttChange.newVal.GetNanoSeconds() / 1e9) << "," <<
                GetDisplayTime(rttChange.time) << endl;
    }
    outfile.close();
}



