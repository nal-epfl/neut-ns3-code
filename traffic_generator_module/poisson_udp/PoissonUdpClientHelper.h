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

class PoissonUdpClientHelper {

private:
    ObjectFactory _factory;

public:
    PoissonUdpClientHelper();

    PoissonUdpClientHelper(Address ip, uint16_t port);
    PoissonUdpClientHelper(Address addr);

    void SetAttribute(std::string name, const AttributeValue &value);

    ApplicationContainer Install(NodeContainer c);

};


#endif //NEUTRALITY_POISSONUDPCLIENTHELPER_H
