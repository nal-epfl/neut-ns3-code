//
// Created by Zeinab Shmeis on 03.06.20.
//

#include "PoissonClientHelper.h"
#include "PoissonClient.h"


PoissonClientHelper::PoissonClientHelper() {
    _factory.SetTypeId (PoissonClient::GetTypeId ());
}

PoissonClientHelper::PoissonClientHelper(Address address, uint16_t port) {
    _factory.SetTypeId (PoissonClient::GetTypeId ());
    SetAttribute ("RemoteAddress", AddressValue(address));
    SetAttribute ("RemotePort", UintegerValue(port));
}

PoissonClientHelper::PoissonClientHelper(Address address) {
    _factory.SetTypeId (PoissonClient::GetTypeId ());
    SetAttribute ("RemoteAddress", AddressValue (address));
}

void PoissonClientHelper::SetAttribute(std::string name, const AttributeValue &value) {
    _factory.Set (name, value);
}

ApplicationContainer PoissonClientHelper::Install(NodeContainer c) {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i) {
        Ptr<Node> node = *i;
        Ptr<PoissonClient> client = _factory.Create<PoissonClient> ();
        node->AddApplication(client);
        apps.Add (client);
    }
    return apps;
}