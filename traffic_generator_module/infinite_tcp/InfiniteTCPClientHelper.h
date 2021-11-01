//
// Created by nal on 01.11.21.
//

#ifndef WEHE_P_TOMOGRAPHY_INFINITETCPCLIENTHELPER_H
#define WEHE_P_TOMOGRAPHY_INFINITETCPCLIENTHELPER_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/application-container.h"

using namespace ns3;
using namespace std;

class InfiniteTCPClientHelper {

private:
    ObjectFactory _factory;

public:
    InfiniteTCPClientHelper();

    InfiniteTCPClientHelper(Address ip, uint16_t port); // here you should set ip and port of the destination
    InfiniteTCPClientHelper(Address addr); // this refers to InetSocketAddress(Address ip, uint16_t port) of the destination Socket

    void SetAttribute(std::string name, const AttributeValue &value);

    ApplicationContainer Install(NodeContainer c);

};


#endif //WEHE_P_TOMOGRAPHY_INFINITETCPCLIENTHELPER_H
