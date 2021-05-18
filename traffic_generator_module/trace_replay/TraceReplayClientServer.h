//
// Created by nal on 05.09.20.
//

#ifndef WEHE_PLUS_TOMOGRAPHY_TRACEREPLAYCLIENTSERVER_H
#define WEHE_PLUS_TOMOGRAPHY_TRACEREPLAYCLIENTSERVER_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

using namespace ns3;
using namespace std;

class TraceReplayClientServer {

private:
    Ptr<Node> _client;
    Ptr<Node> _server;

    string _protocol;
    string _clientTracePath, _serverTracePath;
    int _clientTrafficTos, _serverTrafficTos;

    bool _enableCwndMonitor = false;
    string _cwndSaveFilename = "";

    static int _appCount;

public:
    TraceReplayClientServer();

    void SetClient(const Ptr<Node> &client);
    void SetServer(const Ptr<Node> &server);
    void SetTracesPath(string clientTracesPath, string serverTracePath);
    void SetProtocol(const string &protocol);
    void SetTrafficsTos(int clientTrafficTos, int serverTrafficTos);
    void EnableCwndMonitor(string saveFilename);

    void Setup(ns3::Time startTime, ns3::Time endTime);

};


#endif //WEHE_PLUS_TOMOGRAPHY_TRACEREPLAYCLIENTSERVER_H
