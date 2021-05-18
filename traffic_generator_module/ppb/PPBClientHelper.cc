//
// Created by nal on 22.09.20.
//

#include "PPBClientHelper.h"

PPBClientHelper::PPBClientHelper (std::string protocol, Address address) {
    m_factory.SetTypeId ("ns3::PPBClient");
    m_factory.Set ("Protocol", StringValue (protocol));
    m_factory.Set ("Remote", AddressValue (address));
}

void PPBClientHelper::SetAttribute (std::string name, const AttributeValue &value) {
    m_factory.Set (name, value);
}

ApplicationContainer PPBClientHelper::Install (Ptr<Node> node) const {
    return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer PPBClientHelper::Install (std::string nodeName) const {
    Ptr<Node> node = Names::Find<Node> (nodeName);
    return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer PPBClientHelper::Install (NodeContainer c) const {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i) {
        apps.Add (InstallPriv (*i));
    }

    return apps;
}

Ptr<Application> PPBClientHelper::InstallPriv (Ptr<Node> node) const {
    Ptr<Application> app = m_factory.Create<Application>();
    node->AddApplication(app);

    return app;
}