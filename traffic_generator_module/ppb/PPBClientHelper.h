//
// Created by nal on 22.09.20.
//

#ifndef WEHE_PLUS_TOMOGRAPHY_PPBCLIENTHELPER_H
#define WEHE_PLUS_TOMOGRAPHY_PPBCLIENTHELPER_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

class PPBClientHelper {

private:

    Ptr<Application> InstallPriv (Ptr<Node> node) const;
    std::string m_protocol;
    Address m_remote;
    ObjectFactory m_factory;

public:

    PPBClientHelper (std::string protocol, Address address);

    void SetAttribute (std::string name, const AttributeValue &value);

    ApplicationContainer Install (NodeContainer c) const;
    ApplicationContainer Install (Ptr<Node> node) const;
    ApplicationContainer Install (std::string nodeName) const;

};


#endif //WEHE_PLUS_TOMOGRAPHY_PPBCLIENTHELPER_H
