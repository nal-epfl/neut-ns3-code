//
// Created by Zeinab Shmeis on 03.06.20.
//

#ifndef NEUTRALITY_PROBESSENDERHELPER_H
#define NEUTRALITY_PROBESSENDERHELPER_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/application-container.h"

#include "../../helper_classes/HelperMethods.h"

using namespace ns3;
using namespace std;
using namespace helper_methods;

class ProbesSenderHelper {

private:
    ObjectFactory _factory;

public:
    explicit ProbesSenderHelper(Address addr); // this refers to InetSocketAddress(Address ip, uint16_t port) of the destination Socket
    ProbesSenderHelper();

    void SetAttribute(std::string name, const AttributeValue &value);

    ApplicationContainer Install(NodeContainer c);

    static ApplicationContainer
    CreatePoissonApplication(const string& appTag, InetSocketAddress sinkAddress, bool isTCP, double lambda, uint32_t pktSize,
                             const string &resultsPath, const Ptr<Node> &node);
    static ApplicationContainer
    CreateConstantProbeApplication(const string& appTag, InetSocketAddress sinkAddress, bool isTCP, double lambda,
                                   uint32_t pktSize, const string &resultsPath, const Ptr<Node> &node);

};


#endif //NEUTRALITY_PROBESSENDERHELPER_H
