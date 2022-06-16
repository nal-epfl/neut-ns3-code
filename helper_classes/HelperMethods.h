//
// Created by nal on 18.01.22.
//

#ifndef WEHE_P_TOMOGRAPHY_HELPERMETHODS_H
#define WEHE_P_TOMOGRAPHY_HELPERMETHODS_H

#include <string>
#include <chrono>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <regex>

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

using namespace std;

class HelperMethods {

    public:
        static vector<string> SplitStr (const string &s, char delim);
        template<class T> static string VectorToString(vector<T> vector, string separator);
        static uint32_t GetSubDirCount(const string& dirPath);
        static string ComputeQueueSize(const string& linkRate, const vector<string>& linksDelay);
        static ns3::Ipv4Address GetNodeIP(ns3::Ptr<ns3::Node> node, uint32_t interface);
        static bool doesPolicerLocationMatch(const string& linkLocation, const string& expectedPolicerLocation);
        static bool isPolicerTypePerFlowPolicer(int policerType);

};


#endif //WEHE_P_TOMOGRAPHY_HELPERMETHODS_H
