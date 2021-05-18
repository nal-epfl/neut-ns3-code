//
// Created by nal on 29.09.20.
//

#include "LossMonitor.h"

LossMonitor::LossMonitor(const Time &startTime, const Time &duration, const uint32_t &txNodeId, const uint32_t &rxNodeId, const string &trafficId) {
    _startTime = startTime;
    _duration = duration;
    _trafficId = trafficId;

    _nbSentPkts = 0;
    _nbReceivedPkts = 0;

    Simulator::Schedule(_startTime, &LossMonitor::Connect, this, txNodeId, rxNodeId);
    Simulator::Schedule(_startTime + _duration, &LossMonitor::Disconnect, this, txNodeId, rxNodeId);
}

void LossMonitor::Connect(uint32_t txNodeId, uint32_t rxNodeId) {
    Config::Connect("/NodeList/" + to_string(txNodeId) + "/$ns3::Ipv4L3Protocol/Tx", MakeCallback(&LossMonitor::RecordIpv4PacketSent, this));
    Config::Connect("/NodeList/" + to_string(rxNodeId) + "/$ns3::Ipv4L3Protocol/Rx", MakeCallback(&LossMonitor::RecordIpv4PacketReceived, this));
}

void LossMonitor::Disconnect(uint32_t txNodeId, uint32_t rxNodeId) {
    Config::Disconnect("/NodeList/" + to_string(txNodeId) + "/$ns3::Ipv4L3Protocol/Tx", MakeCallback(&LossMonitor::RecordIpv4PacketSent, this));
//    Config::Disconnect("/NodeList/" + to_string(rxNodeId) + "/$ns3::Ipv4L3Protocol/Rx", MakeCallback(&LossMonitor::RecordIpv4PacketReceived, this));
}

void LossMonitor::AddDestination(Ipv4Address destination) {
    _destinations.insert(destination);
}

void LossMonitor::RecordIpv4PacketSent(string context, Ptr<Packet const> packet, Ptr<Ipv4> ipv4, uint32_t interface) {
    PacketKey* packetKey = PacketKey::Packet2PacketKey(packet);
    if(_destinations.count(packetKey->GetDstIp())) {
        _nbSentPkts++;
    }
}

void LossMonitor::RecordIpv4PacketReceived(string context, Ptr<Packet const> packet, Ptr<Ipv4> ipv4, uint32_t interface) {
    PacketKey* packetKey = PacketKey::Packet2PacketKey(packet);
    if(_destinations.count(packetKey->GetDstIp())) {
        _nbReceivedPkts++;
    }
}

void LossMonitor::DisplayStats() {
    cout << "Stats for " << _trafficId << ":" << endl;
    cout << "nb sent packets = " << _nbSentPkts << ", and nb received packets = " << _nbReceivedPkts << ", ";
    double nbLostPkts = _nbSentPkts - _nbReceivedPkts;
    cout << "so loss ratio = " << ((nbLostPkts / _nbSentPkts) * 100) << endl;
}
