//
// Created by nal on 25.10.21.
//

#include "MeasurementReplaySenderHelper.h"
#include "MeasurementReplaySender.h"

MeasurementReplaySenderHelper::MeasurementReplaySenderHelper() {
    _factory.SetTypeId (MeasurementReplaySender::GetTypeId ());
}

MeasurementReplaySenderHelper::MeasurementReplaySenderHelper(Address address) {
    _factory.SetTypeId (MeasurementReplaySender::GetTypeId ());
    SetAttribute ("RemoteAddress", AddressValue (address));
}

void MeasurementReplaySenderHelper::SetAttribute(std::string name, const AttributeValue &value) {
    _factory.Set (name, value);
}

ApplicationContainer MeasurementReplaySenderHelper::Install(NodeContainer c) {
    ApplicationContainer apps;
    for (auto i = c.Begin (); i != c.End (); ++i) {
        Ptr<Node> node = *i;
        Ptr<MeasurementReplaySender> client = _factory.Create<MeasurementReplaySender> ();
        node->AddApplication(client);
        apps.Add (client);
    }
    return apps;
}

ApplicationContainer
MeasurementReplaySenderHelper::CreateMeasurementReplayApplication(
        const string& appTag, InetSocketAddress sinkAddress, bool isTCP,
        const string &traceFile, const string &resultsPath, const Ptr<Node> &node) {
    MeasurementReplaySenderHelper helper(sinkAddress);
    helper.SetAttribute("AppTag", StringValue(appTag));
    helper.SetAttribute("Protocol", StringValue(GetSocketFactory(isTCP)));
    helper.SetAttribute("TraceFile", StringValue(traceFile));
    helper.SetAttribute("EnableCwndMonitor", BooleanValue(isTCP));
    helper.SetAttribute("ResultsFolder", StringValue(resultsPath));
    return helper.Install(node);
}