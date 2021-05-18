//
// Created by Zeinab Shmeis on 28.05.20.
//

#include "PacketMonitor.h"

#include <utility>

PacketMonitorEvent::PacketMonitorEvent(PacketKey *key) : _key(key) {}

void PacketMonitorEvent::SetSent() {
    _sentTime = ns3::Simulator::Now();
}

void PacketMonitorEvent::SetReceived() {
    _isReceived = true;
    _receivedTime = ns3::Simulator::Now();
}

PacketKey *PacketMonitorEvent::GetPacketKey() const { return _key; }

const Time PacketMonitorEvent::GetSentTime() const { return _sentTime; }

bool PacketMonitorEvent::IsReceived() const { return _isReceived; }

const Time PacketMonitorEvent::GetReceivedTime() const {  return _receivedTime; }

ostream &operator<<(ostream &os, const PacketMonitorEvent &event) {
    os << "_key: " << *(event._key) << " _sentTime: " << event._sentTime << " _isReceived: " << event._isReceived
       << " _receivedTime: " << event._receivedTime;
    return os;
}

ostream &operator<<(ostream &os, const SrcDstPair &pair) {
    os << "src: " << pair.src << " dst: " << pair.dst;
    return os;
}


PacketMonitor::PacketMonitor(const Time &startTime, const Time &duration, const uint32_t &txNodeId, const uint32_t &rxNodeId, const string &trafficId) {
    _startTime = startTime;
    _duration = duration;
    _trafficId = trafficId;

    Simulator::Schedule(_startTime, &PacketMonitor::Connect, this, txNodeId, rxNodeId);
    Simulator::Schedule(_startTime + _duration, &PacketMonitor::Disconnect, this, txNodeId, rxNodeId);
}

void PacketMonitor::Connect(uint32_t txNodeId, uint32_t rxNodeId) {
    Config::Connect("/NodeList/" + to_string(txNodeId) + "/$ns3::Ipv4L3Protocol/Tx", MakeCallback(&PacketMonitor::RecordIpv4PacketSent, this));
    Config::Connect("/NodeList/" + to_string(rxNodeId) + "/$ns3::Ipv4L3Protocol/Rx", MakeCallback(&PacketMonitor::RecordIpv4PacketReceived, this));
//    Config::Connect("/NodeList/" + to_string(rxNodeId) + "/$ns3::Ipv4L3Protocol/LocalDeliver", MakeCallback(&PacketMonitor::RecordIpv4PacketLocalDeliver, this));
}

void PacketMonitor::Disconnect(uint32_t txNodeId, uint32_t rxNodeId) {
    Config::Disconnect("/NodeList/" + to_string(txNodeId) + "/$ns3::Ipv4L3Protocol/Tx", MakeCallback(&PacketMonitor::RecordIpv4PacketSent, this));
//    Config::Disconnect("/NodeList/" + to_string(rxNodeId) + "/$ns3::Ipv4L3Protocol/Rx", MakeCallback(&PacketMonitor::RecordIpv4PacketReceived, this));
}

void PacketMonitor::AddAppKey(Ipv4Address srcIp, Ipv4Address dstIp, uint16_t dstPort) {
    _appsKey.insert(AppKey(srcIp, dstIp, dstPort));
}

void PacketMonitor::RecordIpv4PacketSent(string context, Ptr<Packet const> packet, Ptr<Ipv4> ipv4, uint32_t interface) {
    PacketKey* packetKey = PacketKey::Packet2PacketKey(packet);
    if(_appsKey.count(AppKey::PacketKey2AppKey(*packetKey))) {
        PacketMonitorEvent *packetEvent = new PacketMonitorEvent(packetKey);
        packetEvent->SetSent();
//        cout << _trafficId << " is sending " << packet->ToString() << endl;
        _recordedPackets[*packetKey] = packetEvent;
    }
}

void PacketMonitor::RecordIpv4PacketReceived(string context, Ptr<Packet const> packet, Ptr<Ipv4> ipv4, uint32_t interface) {
    PacketKey* packetKey = PacketKey::Packet2PacketKey(packet);
    if(_appsKey.count(AppKey::PacketKey2AppKey(*packetKey))) {
        auto packetKeyEventPair = _recordedPackets.find(*packetKey);
        if (packetKeyEventPair != _recordedPackets.end()) {
//            cout << _trafficId << " have received " << packet->ToString() << endl;
            packetKeyEventPair->second->SetReceived();
        }
    }
}

void PacketMonitor::RecordIpv4PacketLocalDeliver(string context, const Ipv4Header& ipv4, Ptr<Packet const> packet, uint32_t interface) {
    PacketKey* packetKey = PacketKey::Packet2PacketKey(packet, ipv4);
    if(_appsKey.count(AppKey::PacketKey2AppKey(*packetKey))) {
        auto packetKeyEventPair = _recordedPackets.find(*packetKey);
        if (packetKeyEventPair != _recordedPackets.end()) {
            packetKeyEventPair->second->SetReceived();
        }
    }
}

void PacketMonitor::SaveRecordedPacketsToCSV(const string& filename) {
    ofstream outfile;
    outfile.open(filename);
    outfile << "SourceIP,DestinationIP,Identification,SourcePort,DestinationPort,PayloadSize,PayloadHash,SentTime,IsReceived,ReceiveTime" << endl;
    for (auto& packetKeyEventPair: _recordedPackets) {
        PacketKey key = packetKeyEventPair.first;
        PacketMonitorEvent* event = packetKeyEventPair.second;

        outfile << key.GetSrcIp() << "," << key.GetDstIp() << "," << key.GetId() << "," << key.GetSrcPort() << "," << key.GetDstPort();
        outfile << "," << key.GetSize() << "," << key.GetPayloadHash();
        outfile << ",";
        outfile << (event->GetSentTime() - _startTime).GetNanoSeconds() << "," << event->IsReceived() << "," << (event->GetReceivedTime() - _startTime).GetNanoSeconds();
        outfile << endl;
    }
    outfile.close();
}

void PacketMonitor::SaveRecordedPacketsCompact(const string& filename) {
    ofstream outfile;
    outfile.open(filename);
    outfile << "SourceIP,DestinationIP,PayloadSize,SentTime,IsReceived" << endl;
    for (auto& packetKeyEventPair: _recordedPackets) {
        PacketKey key = packetKeyEventPair.first;
        PacketMonitorEvent* event = packetKeyEventPair.second;

        outfile << key.GetSrcIp() << "," << key.GetDstIp() << "," << key.GetSize();
        outfile << ",";
        outfile << (event->GetSentTime() - _startTime).GetNanoSeconds() << "," << event->IsReceived();
        outfile << endl;
    }
    outfile.close();
}

void PacketMonitor::SaveRecordedPacketsFor1Path(const string& filename) {
    ofstream outfile;
    outfile.open(filename);
    outfile << "SourcePort,DestinationPort,PayloadSize,SentTime,IsReceived,ReceiveTime" << endl;
    for (auto& packetKeyEventPair: _recordedPackets) {
        PacketKey key = packetKeyEventPair.first;
        PacketMonitorEvent* event = packetKeyEventPair.second;

        outfile << key.GetSrcPort() << "," << key.GetDstPort() << "," << key.GetSize();
        outfile << ",";
        outfile << (event->GetSentTime() - _startTime).GetNanoSeconds() << "," << event->IsReceived() << "," << (event->GetReceivedTime() - _startTime).GetNanoSeconds();
        outfile << endl;
    }
    outfile.close();
}

void PacketMonitor::DisplayStats() {
    cout << "Stats for " << _trafficId << ":" << endl;
    int nb_sent_pkts = 0; int nb_lost_pkts = 0;
    for (auto& packetKeyEventPair: _recordedPackets) {
        nb_sent_pkts++;
        PacketMonitorEvent* event = packetKeyEventPair.second;
        if(event->IsReceived() == 0) { nb_lost_pkts++; }
    }
    cout << "nb_sent_pkts = " << nb_sent_pkts << ", and nb_lost_pkts = " << nb_lost_pkts << ", ";
    cout << "so loss ratio = " << ((nb_lost_pkts * 1.0 / nb_sent_pkts) * 100) << endl;
}
