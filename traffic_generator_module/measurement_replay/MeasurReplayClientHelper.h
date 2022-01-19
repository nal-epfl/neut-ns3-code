//
// Created by nal on 25.10.21.
//

#ifndef WEHE_P_TOMOGRAPHY_NEW_MEASURREPLAYCLIENTHELPER_H
#define WEHE_P_TOMOGRAPHY_NEW_MEASURREPLAYCLIENTHELPER_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/application-container.h"

using namespace ns3;
using namespace std;


class MeasurReplayClientHelper {

private:
    ObjectFactory _factory;

public:
    MeasurReplayClientHelper();

    MeasurReplayClientHelper(Address ip, uint16_t port);

    MeasurReplayClientHelper(Address addr);

    void SetAttribute(std::string name, const AttributeValue &value);

    ApplicationContainer Install(NodeContainer c);

    static ApplicationContainer
    CreateMeasurementReplayApplication(InetSocketAddress sinkAddress, bool isTCP, const string &traceFile,
                                       const string &resultsPath, const Ptr<Node> &node);
};


#endif //WEHE_P_TOMOGRAPHY_NEW_MEASURREPLAYCLIENTHELPER_H
