//
// Created by nal on 07.02.21.
//

#ifndef WEHE_P_TOMOGRAPHY_WEHECLIENT_H
#define WEHE_P_TOMOGRAPHY_WEHECLIENT_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "../../monitors_module/CwndMonitor.h"
#include "WeheCS_Utility.h"

using namespace ns3;
using namespace std;

class WeheClient {

public:

    virtual void LoadTrace(vector<WeheTraceItem> &traceItems) = 0;
    virtual void SetResultsFolder(string resultsFolder) = 0;
    virtual void SetDscp(int dscp) = 0;
    virtual void EnableCwndMonitor() = 0;

    virtual void StartApplication() = 0;
    virtual void StopApplication() = 0;


};


#endif //WEHE_P_TOMOGRAPHY_WEHECLIENT_H
