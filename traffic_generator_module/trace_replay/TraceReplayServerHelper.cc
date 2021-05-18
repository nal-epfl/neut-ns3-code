//
// Created by nal on 27.04.21.
//

#include "TraceReplayServer.h"
#include "TraceReplayServerHelper.h"

TraceReplayServerHelper::TraceReplayServerHelper() {
    _factory.SetTypeId (TraceReplayServer::GetTypeId ());
}

TraceReplayServerHelper::TraceReplayServerHelper(Address address) {
    _factory.SetTypeId (TraceReplayServer::GetTypeId ());
    SetAttribute ("ServerAddress", AddressValue (address));
}

void TraceReplayServerHelper::SetAttribute(std::string name, const AttributeValue &value) {
    _factory.Set (name, value);
}

ApplicationContainer TraceReplayServerHelper::Install(NodeContainer c) {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i) {
        Ptr<Node> node = *i;
        Ptr<TraceReplayServer> server = _factory.Create<TraceReplayServer> ();
        node->AddApplication(server);
        apps.Add (server);
    }
    return apps;
}