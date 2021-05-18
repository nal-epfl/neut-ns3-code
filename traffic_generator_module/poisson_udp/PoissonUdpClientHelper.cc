//
// Created by Zeinab Shmeis on 03.06.20.
//

#include "PoissonUdpClientHelper.h"
#include "PoissonUdpClient.h"


PoissonUdpClientHelper::PoissonUdpClientHelper() {
    _factory.SetTypeId (PoissonUdpClient::GetTypeId ());
}

PoissonUdpClientHelper::PoissonUdpClientHelper(Address address, uint16_t port) {
    _factory.SetTypeId (PoissonUdpClient::GetTypeId ());
    SetAttribute ("RemoteAddress", AddressValue(address));
    SetAttribute ("RemotePort", UintegerValue(port));
}

PoissonUdpClientHelper::PoissonUdpClientHelper(Address address) {
    _factory.SetTypeId (PoissonUdpClient::GetTypeId ());
    SetAttribute ("RemoteAddress", AddressValue (address));
}

void PoissonUdpClientHelper::SetAttribute(std::string name, const AttributeValue &value) {
    _factory.Set (name, value);
}

ApplicationContainer PoissonUdpClientHelper::Install(NodeContainer c) {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i) {
        Ptr<Node> node = *i;
        Ptr<PoissonUdpClient> client = _factory.Create<PoissonUdpClient> ();
        node->AddApplication(client);
        apps.Add (client);
    }
    return apps;
}