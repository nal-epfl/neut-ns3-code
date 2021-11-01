//
// Created by nal on 25.10.21.
//

#include "MeasurReplayClientHelper.h"
#include "MeasurReplayClient.h"

MeasurReplayClientHelper::MeasurReplayClientHelper() {
    _factory.SetTypeId (MeasurReplayClient::GetTypeId ());
}

MeasurReplayClientHelper::MeasurReplayClientHelper(Address address, uint16_t port) {
    _factory.SetTypeId (MeasurReplayClient::GetTypeId ());
    SetAttribute ("RemoteAddress", AddressValue(address));
    SetAttribute ("RemotePort", UintegerValue(port));
}

MeasurReplayClientHelper::MeasurReplayClientHelper(Address address) {
    _factory.SetTypeId (MeasurReplayClient::GetTypeId ());
    SetAttribute ("RemoteAddress", AddressValue (address));
}

void MeasurReplayClientHelper::SetAttribute(std::string name, const AttributeValue &value) {
    _factory.Set (name, value);
}

ApplicationContainer MeasurReplayClientHelper::Install(NodeContainer c) {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i) {
        Ptr<Node> node = *i;
        Ptr<MeasurReplayClient> client = _factory.Create<MeasurReplayClient> ();
        node->AddApplication(client);
        apps.Add (client);
    }
    return apps;
}