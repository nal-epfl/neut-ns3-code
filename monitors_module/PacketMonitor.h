//
// Created by Zeinab Shmeis on 28.05.20.
//

#ifndef NEUTRALITY_PACKETMONITOR_H
#define NEUTRALITY_PACKETMONITOR_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

#include "PacketKey.h"
#include "AppKey.h"

#include <ostream>
#include <unordered_map>

using namespace ns3;
using namespace std;

struct PacketMonitorEvent {

private:
    PacketKey* _key;
    ns3::Time _sentTime;
    bool _isReceived = false;
    ns3::Time _receivedTime;

public:
    explicit PacketMonitorEvent(PacketKey *key);

    [[nodiscard]] PacketKey* GetPacketKey() const;
    [[nodiscard]] Time GetSentTime() const;
    [[nodiscard]] bool IsReceived() const;
    [[nodiscard]] Time GetReceivedTime() const;

    void SetSent();
    void SetReceived();

    friend ostream &operator<<(ostream &os, const PacketMonitorEvent &event);
};

class PacketMonitor {

private:
    ns3::Time _startTime = Seconds(0);
    ns3::Time _duration = Seconds(0);

    std::string _monitorTag;
    set<AppKey> _appsKey;

    std::unordered_map<PacketKey, PacketMonitorEvent*, PacketKeyHash> _recordedPackets;

    void Connect(uint32_t txNodeId, uint32_t rxNodeId);
    void Disconnect(uint32_t txNodeId, uint32_t rxNodeId);

    void RecordIpv4PacketSent(Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface);
    void RecordIpv4PacketReceived(Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface);

public:
    PacketMonitor(const Time &startTime, const Time &duration, const Ptr<Node> &txNode, const Ptr<Node> &rxNode, const string &monitorTag);

    void AddAppKey(AppKey appKey);

    void SaveRecordedPacketsToCSV(const string& filename);
    void SaveRecordedPacketsCompact(const string &filename);
    void DisplayStats();
};

#endif //NEUTRALITY_PACKETMONITOR_H
