//
// Created by Zeinab Shmeis on 29.05.20.
//

#ifndef NEUTRALITY_PACKETKEY_H
#define NEUTRALITY_PACKETKEY_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

using namespace ns3;
using namespace std;

class PacketKey {

private:
    Ipv4Address _srcIp;
    Ipv4Address _dstIp;
    uint16_t _id;
    uint16_t _srcPort;
    uint16_t _dstPort;
    uint32_t _size;
    size_t _payloadHash;

public:
    PacketKey(const Ipv4Address &srcIp, const Ipv4Address &dstIp, uint16_t id, uint16_t srcPort, uint16_t dstPort,
                uint32_t size, size_t payloadHash);

    [[nodiscard]] Ipv4Address GetSrcIp() const;
    [[nodiscard]] Ipv4Address GetDstIp() const;
    [[nodiscard]] uint16_t GetId() const;
    [[nodiscard]] uint16_t GetSrcPort() const;
    [[nodiscard]] uint16_t GetDstPort() const;
    [[nodiscard]] uint32_t GetSize() const;
    [[nodiscard]] size_t GetPayloadHash() const;

    bool operator<(const PacketKey &rhs) const;
    bool operator>(const PacketKey &rhs) const;
    bool operator<=(const PacketKey &rhs) const;
    bool operator>=(const PacketKey &rhs) const;
    bool operator==(const PacketKey &rhs) const;
    bool operator!=(const PacketKey &rhs) const;

    friend ostream &operator<<(ostream &os, const PacketKey &event);

    static PacketKey* Packet2PacketKey(Ptr<const Packet> packet);
    static PacketKey *Packet2PacketKey(Ptr<const Packet> packet, const Ipv4Header& ipHeader);
};

struct PacketKeyHash {
    std::size_t operator()(PacketKey const& packetKey) const noexcept;
};


#endif //NEUTRALITY_PACKETKEY_H
