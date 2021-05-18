//
// Created by nal on 07.09.20.
//

#include "ns3/applications-module.h"
#include "PPBClientHelper.h"

#include "PPBBidirectional.h"

PPBBidirectional::PPBBidirectional() {

}

void PPBBidirectional::SetHosts(const Ptr<Node> &host1, const Ptr<Node> &host2) {
    _host1 = host1;
    _host2 = host2;
}

void PPBBidirectional::SetPktSize(uint32_t pktSize) {
    _pktSize = pktSize;
}

void PPBBidirectional::SetUpPPBParams(PPBSettings *UpPPBParams) {
    _upPPBParams = UpPPBParams;
}

void PPBBidirectional::SetDownPPBParams(PPBSettings *downPPBParams) {
    _downPPBParams = downPPBParams;
}

void PPBBidirectional::Setup(ns3::Time startTime, ns3::Time endTime) {
    // helper variables
    Ipv4Address host2Address = _host2->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    Ipv4Address host1Address = _host1->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    int sinkPort = 3000;

    // In the upward direction (host1->host2)
    Address sinkAddressUp = Address(InetSocketAddress(host2Address, sinkPort));
    Ptr<Socket> sinkSocketUp = Socket::CreateSocket(_host2, UdpSocketFactory::GetTypeId());
    sinkSocketUp->Bind(sinkAddressUp);

    PPBClientHelper ppbpHelperUp("ns3::UdpSocketFactory", sinkAddressUp);
    ppbpHelperUp.SetAttribute("BurstIntensity", DataRateValue (_upPPBParams->GetBurstIntensity()));
    ppbpHelperUp.SetAttribute("MeanBurstArrivals", DoubleValue (_upPPBParams->GetMeanBurstArrivals()));
    ppbpHelperUp.SetAttribute("MeanBurstTimeLength", DoubleValue(_upPPBParams->GetMeanBurstTimeLength()));
    ppbpHelperUp.SetAttribute("H", DoubleValue(_upPPBParams->GetHurstParameter()));
    ApplicationContainer ppbpAppUp = ppbpHelperUp.Install(_host1);
    ppbpAppUp.Start(startTime);
    ppbpAppUp.Stop(endTime);


    // In the downward direction (dst->src)
    Address sinkAddressDown = Address(InetSocketAddress(host1Address, sinkPort));
    Ptr<Socket> sinkSocketDown = Socket::CreateSocket(_host1, UdpSocketFactory::GetTypeId());
    sinkSocketDown->Bind(sinkAddressDown);

    PPBClientHelper ppbpHelperDown("ns3::UdpSocketFactory", sinkAddressDown);
    ppbpHelperDown.SetAttribute("BurstIntensity", DataRateValue (_downPPBParams->GetBurstIntensity()));
    ppbpHelperDown.SetAttribute("MeanBurstArrivals", DoubleValue (_downPPBParams->GetMeanBurstArrivals()));
    ppbpHelperDown.SetAttribute("MeanBurstTimeLength", DoubleValue(_downPPBParams->GetMeanBurstTimeLength()));
    ppbpHelperDown.SetAttribute("H", DoubleValue(_downPPBParams->GetHurstParameter()));
    ApplicationContainer ppbpAppDown = ppbpHelperDown.Install(_host2);
    ppbpAppDown.Start(startTime);
    ppbpAppDown.Stop(endTime);

}
