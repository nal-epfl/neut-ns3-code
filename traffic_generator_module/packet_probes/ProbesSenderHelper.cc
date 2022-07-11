//
// Created by Zeinab Shmeis on 03.06.20.
//

#include "ProbesSenderHelper.h"
#include "ProbesSender.h"

ProbesSenderHelper::ProbesSenderHelper() {
    _factory.SetTypeId (ProbesSender::GetTypeId ());
}

ProbesSenderHelper::ProbesSenderHelper(Address address) {
    _factory.SetTypeId (ProbesSender::GetTypeId ());
    SetAttribute ("RemoteAddress", AddressValue (address));
}

void ProbesSenderHelper::SetAttribute(std::string name, const AttributeValue &value) {
    _factory.Set (name, value);
}

ApplicationContainer ProbesSenderHelper::Install(NodeContainer c) {
    ApplicationContainer apps;
    for (auto i = c.Begin (); i != c.End (); ++i) {
        Ptr<Node> node = *i;
        Ptr<ProbesSender> client = _factory.Create<ProbesSender> ();
        node->AddApplication(client);
        apps.Add (client);
    }
    return apps;
}

ApplicationContainer
ProbesSenderHelper::CreatePoissonApplication(
        const string& appTag, InetSocketAddress sinkAddress, bool isTCP, double lambda, uint32_t pktSize,
        const string &resultsPath, const Ptr<Node> &node) {
    ProbesSenderHelper helper(sinkAddress);
    helper.SetAttribute("AppTag", StringValue(appTag));
    helper.SetAttribute("Protocol", StringValue(GetSocketFactory(isTCP)));
    helper.SetAttribute("Interval", StringValue("ns3::ExponentialRandomVariable[Mean=" + to_string(lambda) + "]"));
    helper.SetAttribute("PacketSize", UintegerValue(pktSize));
    helper.SetAttribute("EnableCwndMonitor", BooleanValue(isTCP));
    helper.SetAttribute("ResultsFolder", StringValue(resultsPath));
    return helper.Install(node);
}

ApplicationContainer
ProbesSenderHelper::CreateConstantProbeApplication(
        const string& appTag, InetSocketAddress sinkAddress, bool isTCP, double lambda, uint32_t pktSize,
        const string &resultsPath, const Ptr<Node> &node) {
    ProbesSenderHelper helper(sinkAddress);
    helper.SetAttribute("AppTag", StringValue(appTag));
    helper.SetAttribute("Protocol", StringValue(GetSocketFactory(isTCP)));
    helper.SetAttribute("Interval", StringValue("ns3::ConstantRandomVariable[Constant=" + to_string(lambda) + "]"));
    helper.SetAttribute("PacketSize", UintegerValue(pktSize));
    helper.SetAttribute("EnableCwndMonitor", BooleanValue(isTCP));
    helper.SetAttribute("ResultsFolder", StringValue(resultsPath));
    return helper.Install(node);
}
