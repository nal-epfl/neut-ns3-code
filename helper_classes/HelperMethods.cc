//
// Created by nal on 18.01.22.
//

#include "HelperMethods.h"

vector<string> HelperMethods::SplitStr (const string &s, char delim) {
    vector<string> result;
    stringstream ss (s);
    string item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}

uint32_t HelperMethods::GetSubDirCount(const string& dirPath) {
    auto dirIter = std::filesystem::directory_iterator(dirPath);
    return count_if(
            begin(dirIter),
            end(dirIter),
            [](auto& entry) { return entry.is_regular_file(); }
    );
}

string HelperMethods::ComputeQueueSize(const string& linkRate, const vector<string>& linksDelay) { // RTT * link_rate
    regex reg("([0-9]*)ms");
    double minRTT = 0.005; // in sec
    for(const string& delay : linksDelay) {
        minRTT += 2 * stoi(regex_replace(delay, reg, "$1")) * 1e-3;
    }
    return to_string(minRTT * (ns3::DataRate(linkRate).GetBitRate() * 0.125)) + "B";
}

ns3::Ipv4Address HelperMethods::GetNodeIP(ns3::Ptr<ns3::Node> node, uint32_t interface) {
    return node->GetObject<ns3::Ipv4>()->GetAddress(interface, 0).GetLocal();
}
