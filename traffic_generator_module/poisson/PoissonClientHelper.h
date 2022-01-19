//
// Created by Zeinab Shmeis on 03.06.20.
//

#ifndef NEUTRALITY_POISSONUDPCLIENTHELPER_H
#define NEUTRALITY_POISSONUDPCLIENTHELPER_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/application-container.h"

using namespace ns3;
using namespace std;

class PoissonClientHelper {

private:
    ObjectFactory _factory;

public:
    PoissonClientHelper();

    PoissonClientHelper(Address ip, uint16_t port); // here you should set ip and port of the destination
    PoissonClientHelper(Address addr); // this refers to InetSocketAddress(Address ip, uint16_t port) of the destination Socket

    void SetAttribute(std::string name, const AttributeValue &value);

    ApplicationContainer Install(NodeContainer c);

    static ApplicationContainer CreatePoissonApplication(InetSocketAddress sinkAddress, bool isTCP, double lambda, uint32_t pktSize, const string& resultsPath, const Ptr<Node>& node);
    static ApplicationContainer CreateConstantProbeApplication(InetSocketAddress sinkAddress, bool isTCP, double lambda, uint32_t pktSize, const string& resultsPath, const Ptr<Node>& node);

};


#endif //NEUTRALITY_POISSONUDPCLIENTHELPER_H
