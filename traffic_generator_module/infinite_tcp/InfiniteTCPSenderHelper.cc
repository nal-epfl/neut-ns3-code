//
// Created by nal on 01.11.21.
//

#include "InfiniteTCPSenderHelper.h"

#include <utility>
#include "InfiniteTCPSender.h"

InfiniteTCPSenderHelper::InfiniteTCPSenderHelper() {
    _factory.SetTypeId (InfiniteTCPSender::GetTypeId ());
}

InfiniteTCPSenderHelper::InfiniteTCPSenderHelper(const Address& address) {
    _factory.SetTypeId (InfiniteTCPSender::GetTypeId ());
    SetAttribute ("RemoteAddress", AddressValue (address));
}

void InfiniteTCPSenderHelper::SetAttribute(const std::string& name, const AttributeValue &value) {
    _factory.Set (name, value);
}

ApplicationContainer InfiniteTCPSenderHelper::Install(const NodeContainer& c) {
    ApplicationContainer apps;
    for (auto i = c.Begin (); i != c.End (); ++i) {
        Ptr<Node> node = *i;
        Ptr<InfiniteTCPSender> client = _factory.Create<InfiniteTCPSender> ();
        node->AddApplication(client);
        apps.Add (client);
    }
    return apps;
}

ApplicationContainer
InfiniteTCPSenderHelper::CreateInfiniteTcpApplication(
        const string& appTag, InetSocketAddress sinkAddress, const string &tcpProtocol, uint32_t pktSize,
        const string &resultsPath, const Ptr<Node> &node, string dataRate) {
    InfiniteTCPSenderHelper helper(sinkAddress);
    helper.SetAttribute("AppTag", StringValue(appTag));
    helper.SetAttribute("TcpProtocol", StringValue(tcpProtocol));
    helper.SetAttribute("PacketSize", UintegerValue(pktSize));
    helper.SetAttribute("EnableCwndMonitor", BooleanValue(true));
    helper.SetAttribute("ResultsFolder", StringValue(resultsPath));
    helper.SetAttribute("MaxSendingRate", DataRateValue(DataRate(std::move(dataRate))));
    return helper.Install(node);
}
