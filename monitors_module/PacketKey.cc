//
// Created by Zeinab Shmeis on 29.05.20.
//

#include "PacketKey.h"

#include "ns3/applications-module.h"
#include "ns3/ppp-header.h"

#include <boost/functional/hash.hpp>


PacketKey::PacketKey(const Ipv4Address &srcIp, const Ipv4Address &dstIp, uint16_t id, uint16_t srcPort,
                     uint16_t dstPort, uint32_t size, size_t payloadHash) : _srcIp(srcIp), _dstIp(dstIp),
                        _id(id), _srcPort(srcPort), _dstPort(dstPort), _size(size), _payloadHash(payloadHash) {}

bool PacketKey::operator<(const PacketKey &rhs) const {
    if (_srcIp < rhs._srcIp) return true;
    if (rhs._srcIp < _srcIp) return false;
    if (_dstIp < rhs._dstIp) return true;
    if (rhs._dstIp < _dstIp) return false;
    if (_srcPort < rhs._srcPort) return true;
    if (rhs._srcPort < _srcPort) return false;
    if (_dstPort < rhs._dstPort) return true;
    if (rhs._dstPort < _dstPort) return false;
    if (_id < rhs._id) return true;
    if (rhs._id < _id) return false;
    return _payloadHash < rhs._payloadHash;
}

bool PacketKey::operator>(const PacketKey &rhs) const {
    return rhs < *this;
}

bool PacketKey::operator<=(const PacketKey &rhs) const {
    return !(rhs < *this);
}

bool PacketKey::operator>=(const PacketKey &rhs) const {
    return !(*this < rhs);
}

bool PacketKey::operator==(const PacketKey &rhs) const {
    return _srcIp == rhs._srcIp &&
           _dstIp == rhs._dstIp &&
           _srcPort == rhs._srcPort &&
           _dstPort == rhs._dstPort &&
           _id == rhs._id &&
           _size == rhs._size &&
           _payloadHash == rhs._payloadHash;
}

bool PacketKey::operator!=(const PacketKey &rhs) const {
    return !(rhs == *this);
}

ostream &operator<<(ostream &os, const PacketKey &key) {
    os << "_srcIp: " << key._srcIp << " _dstIp: " << key._dstIp << " _srcPort: " << key._srcPort << " _dstPort: "
       << key._dstPort << " _id: " << key._id << " _payloadHash:" << key._payloadHash;
    return os;
}

Ipv4Address PacketKey::GetSrcIp() const { return _srcIp; }

Ipv4Address PacketKey::GetDstIp() const { return _dstIp; }

uint16_t PacketKey::GetId() const { return _id; }

uint16_t PacketKey::GetSrcPort() const { return _srcPort; }

uint16_t PacketKey::GetDstPort() const { return _dstPort; }

uint32_t PacketKey::GetSize() const { return _size; }

size_t PacketKey::GetPayloadHash() const { return _payloadHash; }

PacketKey* PacketKey::Packet2PacketKey(Ptr<const Packet> packet) {
    const Ptr<Packet> &pktCopy = packet->Copy();

    Ipv4Header ipHeader;
    pktCopy->RemoveHeader(ipHeader);
    Ipv4Address srcIp = ipHeader.GetSource();
    Ipv4Address dstIp = ipHeader.GetDestination();
    uint16_t id = ipHeader.GetIdentification();

    uint16_t srcPort = 0;
    uint16_t dstPort = 0;
    hash<string> hasher;
    uint32_t pktSize;
    size_t payloadHash = 0;
    if(ipHeader.GetProtocol() == 6) {
        TcpHeader tcpHeader;
        pktCopy->RemoveHeader(tcpHeader);
        srcPort = tcpHeader.GetSourcePort();
        dstPort = tcpHeader.GetDestinationPort();
        pktSize = pktCopy->GetSize();
        payloadHash = hasher(pktCopy->ToString());
        pktCopy->AddHeader(tcpHeader);
    }
    else if (ipHeader.GetProtocol() == 17) {
        UdpHeader udpHeader;
        pktCopy->RemoveHeader(udpHeader);
        srcPort = udpHeader.GetSourcePort();
        dstPort = udpHeader.GetDestinationPort();
        pktSize = pktCopy->GetSize();
        payloadHash = hasher(pktCopy->ToString());
        pktCopy->AddHeader(udpHeader);
    }

    pktCopy->AddHeader(ipHeader);

    return new PacketKey(srcIp, dstIp, id, srcPort, dstPort, pktSize, payloadHash);
}

PacketKey* PacketKey::Packet2PacketKey(Ptr<const Packet> packet, const Ipv4Header& ipHeader) {
    const Ptr<Packet> &pktCopy = packet->Copy();

//    pktCopy->RemoveHeader(ipHeader);
    Ipv4Address srcIp = ipHeader.GetSource();
    Ipv4Address dstIp = ipHeader.GetDestination();
    uint16_t id = ipHeader.GetIdentification();

    uint16_t srcPort = 0;
    uint16_t dstPort = 0;
    hash<string> hasher;
    uint32_t pktSize;
    size_t payloadHash = 0;
    if(ipHeader.GetProtocol() == 6) {
        TcpHeader tcpHeader;
        pktCopy->RemoveHeader(tcpHeader);
        srcPort = tcpHeader.GetSourcePort();
        dstPort = tcpHeader.GetDestinationPort();
        pktSize = pktCopy->GetSize();
        payloadHash = hasher(pktCopy->ToString());
        pktCopy->AddHeader(tcpHeader);
    }
    else if (ipHeader.GetProtocol() == 17) {
        UdpHeader udpHeader;
        pktCopy->RemoveHeader(udpHeader);
        srcPort = udpHeader.GetSourcePort();
        dstPort = udpHeader.GetDestinationPort();
        pktSize = pktCopy->GetSize();
        payloadHash = hasher(pktCopy->ToString());
        pktCopy->AddHeader(udpHeader);
    }

//    pktCopy->AddHeader(ipHeader);

    return new PacketKey(srcIp, dstIp, id, srcPort, dstPort, pktSize, payloadHash);;
}


std::size_t PacketKeyHash::operator()(PacketKey const &packetKey) const noexcept {
    std::size_t seed = 0x315E4139;
    Ipv4AddressHash ipHash;
    boost::hash_combine(seed, ipHash(packetKey.GetSrcIp()));
    boost::hash_combine(seed, ipHash(packetKey.GetDstIp()));
    boost::hash_combine(seed, static_cast<std::size_t>(packetKey.GetId()));
    boost::hash_combine(seed, static_cast<std::size_t>(packetKey.GetSrcPort()));
    boost::hash_combine(seed, static_cast<std::size_t>(packetKey.GetDstPort()));
    boost::hash_combine(seed, packetKey.GetPayloadHash());
    return seed;
}
