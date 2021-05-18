//
// Created by nal on 13.10.20.
//

#ifndef WEHE_PLUS_TOMOGRAPHY_PBCLIENTHELPER_H
#define WEHE_PLUS_TOMOGRAPHY_PBCLIENTHELPER_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

class PBClientHelper {

private:

    Ptr<Application> InstallPriv (Ptr<Node> node) const;
    std::string m_protocol;
    Address m_remote;
    ObjectFactory m_factory;

public:

    PBClientHelper (std::string protocol, Address address);

    void SetAttribute (std::string name, const AttributeValue &value);

    ApplicationContainer Install (NodeContainer c) const;
    ApplicationContainer Install (Ptr<Node> node) const;
    ApplicationContainer Install (std::string nodeName) const;

};


#endif //WEHE_PLUS_TOMOGRAPHY_PBCLIENTHELPER_H
