//
// Created by nal on 22.03.21.
//

#include <boost/functional/hash.hpp>
#include "AppKey.h"

AppKey::AppKey(const Ipv4Address &srcIp, const Ipv4Address &dstIp, uint16_t dstPort) : _srcIp(srcIp), _dstIp(dstIp), _dstPort(dstPort){}

bool AppKey::operator<(const AppKey &rhs) const {
    if (_srcIp < rhs._srcIp) return true;
    if (rhs._srcIp < _srcIp) return false;
    if (_dstIp < rhs._dstIp) return true;
    if (rhs._dstIp < _dstIp) return false;
    return _dstPort < rhs._dstPort;
}

bool AppKey::operator>(const AppKey &rhs) const {
    return rhs < *this;
}

bool AppKey::operator<=(const AppKey &rhs) const {
    return !(rhs < *this);
}

bool AppKey::operator>=(const AppKey &rhs) const {
    return !(*this < rhs);
}

bool AppKey::operator==(const AppKey &rhs) const {
    return _srcIp == rhs._srcIp && _dstIp == rhs._dstIp && _dstPort == rhs._dstPort;
}

bool AppKey::operator!=(const AppKey &rhs) const {
    return !(rhs == *this);
}

ostream &operator<<(ostream &os, const AppKey &key) {
    os << "_srcIp: " << key._srcIp << " _dstIp: " << key._dstIp << " _dstPort: " << key._dstPort;
    return os;
}

Ipv4Address AppKey::GetSrcIp() const { return _srcIp; }

Ipv4Address AppKey::GetDstIp() const { return _dstIp; }

uint16_t AppKey::GetDstPort() const { return _dstPort; }

bool AppKey::belongsToApp(const PacketKey &packetKey) {
    return _srcIp == packetKey.GetSrcIp() && _dstIp == packetKey.GetDstIp() && _dstPort == packetKey.GetDstPort();
}

AppKey AppKey::PacketKey2AppKey(const PacketKey &packetKey) {
    return AppKey(packetKey.GetSrcIp(), packetKey.GetDstIp(), packetKey.GetSrcPort()); // TODO change this in case using tcp flows
//    return AppKey(packetKey.GetSrcIp(), packetKey.GetDstIp(), packetKey.GetDstPort());
}


std::size_t AppKeyHash::operator()(AppKey const &AppKey) const noexcept {
    std::size_t seed = 0x315E4138;
    Ipv4AddressHash ipHash;
    boost::hash_combine(seed, ipHash(AppKey.GetSrcIp()));
    boost::hash_combine(seed, ipHash(AppKey.GetDstIp()));
    boost::hash_combine(seed, static_cast<std::size_t>(AppKey.GetDstPort()));
    return seed;
}
