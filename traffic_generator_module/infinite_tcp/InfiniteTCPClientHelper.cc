//
// Created by nal on 01.11.21.
//

#include "InfiniteTCPClientHelper.h"

#include <utility>
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

ApplicationContainer
InfiniteTCPClientHelper::CreateInfiniteTcpApplication(InetSocketAddress sinkAddress, const string &tcpProtocol,
                                                      uint32_t pktSize, const string &resultsPath,
                                                      const Ptr<Node> &node, string dataRate) {
    InfiniteTCPClientHelper helper(sinkAddress);
    helper.SetAttribute("TcpProtocol", StringValue(tcpProtocol));
    helper.SetAttribute("PacketSize", UintegerValue(pktSize));
    helper.SetAttribute("EnableCwndMonitor", BooleanValue(true));
    helper.SetAttribute("ResultsFolder", StringValue(resultsPath));
    helper.SetAttribute("MaxSendingRate", DataRateValue(DataRate(std::move(dataRate))));
    return helper.Install(node);
}
