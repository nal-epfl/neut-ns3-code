//
// Created by nal on 31.08.20.
//

#ifndef WEHE_PLUS_TOMOGRAPHY_TRACEREPLAYCLIENTHELPER_H
#define WEHE_PLUS_TOMOGRAPHY_TRACEREPLAYCLIENTHELPER_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/application-container.h"

using namespace ns3;
using namespace std;


class TraceReplayClientHelper {

private:
    ObjectFactory _factory;

public:
    TraceReplayClientHelper();

    TraceReplayClientHelper(Address ip, uint16_t port);
    TraceReplayClientHelper(Address addr);

    void SetAttribute(std::string name, const AttributeValue &value);

    ApplicationContainer Install(NodeContainer c);

};


#endif //WEHE_PLUS_TOMOGRAPHY_TRACEREPLAYCLIENTHELPER_H
