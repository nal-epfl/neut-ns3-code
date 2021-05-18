//
// Created by nal on 31.08.20.
//

#include "TraceReplayClientHelper.h"
#include "TraceReplayClient.h"


TraceReplayClientHelper::TraceReplayClientHelper() {
    _factory.SetTypeId (TraceReplayClient::GetTypeId ());
}

TraceReplayClientHelper::TraceReplayClientHelper(Address address, uint16_t port) {
    _factory.SetTypeId (TraceReplayClient::GetTypeId ());
    SetAttribute ("RemoteAddress", AddressValue(address));
    SetAttribute ("RemotePort", UintegerValue(port));
}

TraceReplayClientHelper::TraceReplayClientHelper(Address address) {
    _factory.SetTypeId (TraceReplayClient::GetTypeId ());
    SetAttribute ("RemoteAddress", AddressValue (address));
}

void TraceReplayClientHelper::SetAttribute(std::string name, const AttributeValue &value) {
    _factory.Set (name, value);
}

ApplicationContainer TraceReplayClientHelper::Install(NodeContainer c) {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i) {
        Ptr<Node> node = *i;
        Ptr<TraceReplayClient> client = _factory.Create<TraceReplayClient> ();
        node->AddApplication(client);
        apps.Add (client);
    }
    return apps;
}