//
// Created by nal on 13.10.20.
//

#include "PBClientHelper.h"

PBClientHelper::PBClientHelper (std::string protocol, Address address) {
    m_factory.SetTypeId ("ns3::PBClient");
    m_factory.Set ("Protocol", StringValue (protocol));
    m_factory.Set ("Remote", AddressValue (address));
}

void PBClientHelper::SetAttribute (std::string name, const AttributeValue &value) {
    m_factory.Set (name, value);
}

ApplicationContainer PBClientHelper::Install (Ptr<Node> node) const {
    return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer PBClientHelper::Install (std::string nodeName) const {
    Ptr<Node> node = Names::Find<Node> (nodeName);
    return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer PBClientHelper::Install (NodeContainer c) const {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i) {
        apps.Add (InstallPriv (*i));
    }

    return apps;
}

Ptr<Application> PBClientHelper::InstallPriv (Ptr<Node> node) const {
    Ptr<Application> app = m_factory.Create<Application>();
    node->AddApplication(app);

    return app;
}