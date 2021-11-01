//
// Created by nal on 01.11.21.
//

#include "InfiniteTCPClientHelper.h"
#include "InfiniteTCPClient.h"

InfiniteTCPClientHelper::InfiniteTCPClientHelper() {
    _factory.SetTypeId (InfiniteTCPClient::GetTypeId ());
}

InfiniteTCPClientHelper::InfiniteTCPClientHelper(Address address, uint16_t port) {
    _factory.SetTypeId (InfiniteTCPClient::GetTypeId ());
    SetAttribute ("RemoteAddress", AddressValue(address));
    SetAttribute ("RemotePort", UintegerValue(port));
}

InfiniteTCPClientHelper::InfiniteTCPClientHelper(Address address) {
    _factory.SetTypeId (InfiniteTCPClient::GetTypeId ());
    SetAttribute ("RemoteAddress", AddressValue (address));
}

void InfiniteTCPClientHelper::SetAttribute(std::string name, const AttributeValue &value) {
    _factory.Set (name, value);
}

ApplicationContainer InfiniteTCPClientHelper::Install(NodeContainer c) {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i) {
        Ptr<Node> node = *i;
        Ptr<InfiniteTCPClient> client = _factory.Create<InfiniteTCPClient> ();
        node->AddApplication(client);
        apps.Add (client);
    }
    return apps;
}
