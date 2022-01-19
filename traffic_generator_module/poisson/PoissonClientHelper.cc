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

ApplicationContainer
PoissonClientHelper::CreatePoissonApplication(InetSocketAddress sinkAddress, bool isTCP, double lambda,
                                              uint32_t pktSize, const string &resultsPath, const Ptr<Node> &node) {
    PoissonClientHelper helper(sinkAddress);
    helper.SetAttribute("Protocol", StringValue((isTCP == 1) ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory"));
    helper.SetAttribute("Interval", StringValue("ns3::ExponentialRandomVariable[Mean=" + to_string(lambda) + "]"));
    helper.SetAttribute("PacketSize", UintegerValue(pktSize));
    helper.SetAttribute("EnableCwndMonitor", BooleanValue(isTCP));
    helper.SetAttribute("ResultsFolder", StringValue(resultsPath));
    return helper.Install(node);
}

ApplicationContainer
PoissonClientHelper::CreateConstantProbeApplication(InetSocketAddress sinkAddress, bool isTCP, double lambda,
                                                    uint32_t pktSize, const string &resultsPath,
                                                    const Ptr<Node> &node) {
    PoissonClientHelper helper(sinkAddress);
    helper.SetAttribute("Protocol", StringValue((isTCP == 1) ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory"));
    helper.SetAttribute("Interval", StringValue("ns3::ConstantRandomVariable[Constant=" + to_string(lambda) + "]"));
    helper.SetAttribute("PacketSize", UintegerValue(pktSize));
    helper.SetAttribute("EnableCwndMonitor", BooleanValue(isTCP));
    helper.SetAttribute("ResultsFolder", StringValue(resultsPath));
    return helper.Install(node);
}
