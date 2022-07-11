//
// Created by nal on 01.11.21.
//

#ifndef WEHE_P_TOMOGRAPHY_INFINITETCPSENDERHELPER_H
#define WEHE_P_TOMOGRAPHY_INFINITETCPSENDERHELPER_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/application-container.h"

using namespace ns3;
using namespace std;

class InfiniteTCPSenderHelper {

private:
    ObjectFactory _factory;

public:
    explicit InfiniteTCPSenderHelper(const Address& addr);
    InfiniteTCPSenderHelper();

    void SetAttribute(const std::string& name, const AttributeValue &value);

    ApplicationContainer Install(const NodeContainer& c);

    static ApplicationContainer
    CreateInfiniteTcpApplication(const string& appTag, InetSocketAddress sinkAddress, const string &tcpProtocol,
                                 uint32_t pktSize, const string &resultsPath, const Ptr<Node> &node, string dataRate);

};


#endif //WEHE_P_TOMOGRAPHY_INFINITETCPSENDERHELPER_H
