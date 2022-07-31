//
// Created by Zeinab Shmeis on 28.05.20.
//

#include "PacketMonitor.h"

PacketMonitorEvent::PacketMonitorEvent(PacketKey *key) : _key(key) {}

void PacketMonitorEvent::SetSent() { _sentTime = ns3::Simulator::Now(); }

void PacketMonitorEvent::SetReceived() {
    _isReceived = true;
    _receivedTime = ns3::Simulator::Now();
}

PacketKey *PacketMonitorEvent::GetPacketKey() const { return _key; }

Time PacketMonitorEvent::GetSentTime() const { return _sentTime; }

bool PacketMonitorEvent::IsReceived() const { return _isReceived; }

Time PacketMonitorEvent::GetReceivedTime() const {  return _receivedTime; }

ostream &operator<<(ostream &os, const PacketMonitorEvent &event) {
    os << "_key: " << *(event._key) << " _sentTime: " << event._sentTime << " _isReceived: " << event._isReceived
       << " _receivedTime: " << event._receivedTime;
    return os;
}


PacketMonitor::PacketMonitor(const Time &startTime, const Time &duration, const Ptr<Node> &txNode, const Ptr<Node> &rxNode, const string &monitorTag) {
    _startTime = startTime;
    _duration = duration;
    _monitorTag = monitorTag;

    Simulator::Schedule(_startTime, &PacketMonitor::Connect, this, txNode->GetId(), rxNode->GetId());
    Simulator::Schedule(_startTime + _duration, &PacketMonitor::Disconnect, this, txNode->GetId(), rxNode->GetId());
}

void PacketMonitor::Connect(uint32_t txNodeId, uint32_t rxNodeId) {
    Config::ConnectWithoutContext("/NodeList/" + to_string(txNodeId) + "/$ns3::Ipv4L3Protocol/Tx", MakeCallback(
            &PacketMonitor::RecordIpv4PacketSent, this));
    Config::ConnectWithoutContext("/NodeList/" + to_string(rxNodeId) + "/$ns3::Ipv4L3Protocol/Rx", MakeCallback(
            &PacketMonitor::RecordIpv4PacketReceived, this));
}

void PacketMonitor::Disconnect(uint32_t txNodeId, uint32_t rxNodeId) {
    Config::DisconnectWithoutContext("/NodeList/" + to_string(txNodeId) + "/$ns3::Ipv4L3Protocol/Tx", MakeCallback(
            &PacketMonitor::RecordIpv4PacketSent, this));

}

void PacketMonitor::AddAppKey(AppKey appKey) {
    _appsKey.insert(appKey);
}

void PacketMonitor::RecordIpv4PacketSent(Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface) {
    PacketKey* packetKey = PacketKey::Packet2PacketKey(packet);
    if(_appsKey.count(AppKey::PacketKey2AppKey(*packetKey))) {
        auto *packetEvent = new PacketMonitorEvent(packetKey);
        packetEvent->SetSent();
        _recordedPackets[*packetKey] = packetEvent;
    }
}

void PacketMonitor::RecordIpv4PacketReceived(Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface) {
    PacketKey* packetKey = PacketKey::Packet2PacketKey(packet);
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
    outfile << "SourcePort,DestinationPort,PayloadSize,SentTime,IsReceived,ReceiveTime" << endl;
    for (auto& packetKeyEventPair: _recordedPackets) {
        PacketKey key = packetKeyEventPair.first;
        PacketMonitorEvent* event = packetKeyEventPair.second;

        outfile << key.GetSrcPort() << "," << key.GetDstPort() << "," << key.GetSize();
        outfile << ",";
        outfile << (event->GetSentTime() - _startTime).GetNanoSeconds();
        outfile << ",";
        outfile << event->IsReceived() << "," << (event->GetReceivedTime() - _startTime).GetNanoSeconds();
        outfile << endl;
    }
    outfile.close();
}

void PacketMonitor::DisplayStats() {
    cout << "Stats for " << _monitorTag << ":" << endl;
    int nb_sent_pkts = 0; int nb_lost_pkts = 0;
    for (auto& packetKeyEventPair: _recordedPackets) {
        nb_sent_pkts++;
        PacketMonitorEvent* event = packetKeyEventPair.second;
        if(event->IsReceived() == 0) { nb_lost_pkts++; }
    }
    cout << "nb_sent_pkts = " << nb_sent_pkts << ", and nb_lost_pkts = " << nb_lost_pkts << ", ";
    cout << "so loss ratio = " << ((nb_lost_pkts * 1.0 / nb_sent_pkts) * 100) << endl;
}
