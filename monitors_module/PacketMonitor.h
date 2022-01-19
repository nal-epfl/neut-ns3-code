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
    [[nodiscard]] const Time GetSentTime() const;
    [[nodiscard]] bool IsReceived() const;
    [[nodiscard]] const Time GetReceivedTime() const;

    void SetSent();
    void SetReceived();

    friend ostream &operator<<(ostream &os, const PacketMonitorEvent &event);
};

struct SrcDstPair {
    Ipv4Address src, dst;

    bool operator <(const SrcDstPair& pair) const {
        if (src < pair.src) return true;
        if (pair.src < src) return false;
        return dst < pair.dst;
    }

    bool operator >(const SrcDstPair& pair) const {
        return pair < *this;
    }

    friend ostream &operator<<(ostream &os, const SrcDstPair &pair);
};

class PacketMonitor {

private:
    ns3::Time _startTime = Seconds(0);
    ns3::Time _duration = Seconds(0);

    std::string _trafficId = "";
    set<AppKey> _appsKey;

    std::unordered_map<PacketKey, PacketMonitorEvent*, PacketKeyHash> _recordedPackets;

    void Connect(uint32_t txNodeId, uint32_t rxNodeId);
    void Disconnect(uint32_t txNodeId, uint32_t rxNodeId);

    void RecordIpv4PacketSent(string context, Ptr<Packet const> packet, Ptr<Ipv4> ipv4, uint32_t interface);
    void RecordIpv4PacketReceived(string context, Ptr<Packet const> packet, Ptr<Ipv4> ipv4, uint32_t interface);
    void RecordIpv4PacketLocalDeliver(string context, const Ipv4Header& ipv4, Ptr<const Packet> packet, uint32_t interface);

public:
    PacketMonitor(const Time &startTime, const Time &duration, const uint32_t &txNodeId, const uint32_t &rxNodeId, const string &trafficId);

    void AddAppKey(Ipv4Address srcIp, Ipv4Address dstIp, uint16_t srcPort, uint16_t dstPort);
    void AddAppKey(Ipv4Address srcIp, Ipv4Address dstIp);
    void AddAppKey(AppKey appKey);

    void SaveRecordedPacketsToCSV(const string& filename);
    void SaveRecordedPacketsCompactByIP(const string& filename);
    void SaveRecordedPacketsCompact(const string &filename);

    void DisplayStats();
};

#endif //NEUTRALITY_PACKETMONITOR_H
