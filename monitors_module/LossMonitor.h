//
// Created by nal on 29.09.20.
//

#ifndef WEHE_PLUS_TOMOGRAPHY_LOSSMONITOR_H
#define WEHE_PLUS_TOMOGRAPHY_LOSSMONITOR_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

#include "PacketKey.h"

#include <ostream>
#include <unordered_map>

using namespace ns3;
using namespace std;

class LossMonitor {

private:
    ns3::Time _startTime = Seconds(0);
    ns3::Time _duration = Seconds(0);

    std::string _trafficId = "";
    set<Ipv4Address> _destinations;

    uint32_t _nbSentPkts;
    uint32_t _nbReceivedPkts;

    void Connect(uint32_t txNodeId, uint32_t rxNodeId);
    void Disconnect(uint32_t txNodeId, uint32_t rxNodeId);

    void RecordIpv4PacketSent(string context, Ptr<Packet const> packet, Ptr<Ipv4> ipv4, uint32_t interface);
    void RecordIpv4PacketReceived(string context, Ptr<Packet const> packet, Ptr<Ipv4> ipv4, uint32_t interface);

public:
    LossMonitor(const Time &startTime, const Time &duration, const uint32_t &txNodeId, const uint32_t &rxNodeId, const string &trafficId);

    void AddDestination(Ipv4Address destination);

    void DisplayStats();
};


#endif //WEHE_PLUS_TOMOGRAPHY_LOSSMONITOR_H
