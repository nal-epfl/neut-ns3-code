//
// Created by nal on 17.02.21.
//

#ifndef WEHE_P_TOMOGRAPHY_WEHECS_UTILITY_H
#define WEHE_P_TOMOGRAPHY_WEHECS_UTILITY_H

#include "ns3/core-module.h"

using namespace ns3;
using namespace std;

enum AppSide {CLIENT, SERVER};

struct WeheTraceItem {
    uint32_t frameNb;
    Time timestamp;
    uint32_t payloadSize;
    AppSide appSide;
    uint32_t preBytesRx;

    string ToString() {
        return "Wehe trace item: frameNb=" + to_string(frameNb) + ", timestamp=" + to_string(timestamp.GetSeconds())
               + ", payloadSize=" + to_string(payloadSize) + ", appSide=" + to_string(appSide)
               + ", preBytesRecv=" + to_string(preBytesRx);
    }
};

struct RxEvent {
    uint32_t bytesRx;
    double rxTime; // in seconds
};

#endif //WEHE_P_TOMOGRAPHY_WEHECS_UTILITY_H
