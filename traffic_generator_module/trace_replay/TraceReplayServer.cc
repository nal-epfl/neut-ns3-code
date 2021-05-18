//
// Created by nal on 27.04.21.
//

#include "TraceReplayServer.h"

NS_LOG_COMPONENT_DEFINE ("TraceReplayServer");

NS_OBJECT_ENSURE_REGISTERED (TraceReplayServer);

TypeId TraceReplayServer::GetTypeId(void) {

    static TypeId tid = TypeId ("ns3::TraceReplayServer")
            .SetParent<Application> ()
            .SetGroupName("Applications")
            .AddConstructor<TraceReplayServer> ()
            .AddAttribute ("ServerAddress",
                           "The Address of the created server socket",
                           AddressValue (),
                           MakeAddressAccessor (&TraceReplayServer::_serverAddress),
                           MakeAddressChecker ())
            .AddAttribute ("Protocol", "the name of the protocol to use to send traffic by the applications",
                           StringValue ("ns3::UdpSocketFactory"),
                           MakeStringAccessor (&TraceReplayServer::_protocol),
                           MakeStringChecker())
    ;
    return tid;
}

TraceReplayServer::TraceReplayServer() {
    NS_LOG_FUNCTION (this);
    _socket = 0;
}

TraceReplayServer::~TraceReplayServer() {
    NS_LOG_FUNCTION (this);
}

void TraceReplayServer::DoDispose(void) {
    NS_LOG_FUNCTION (this);
    Application::DoDispose();
}

void TraceReplayServer::StartApplication() {
    if (_socket == 0)     {
        TypeId tid = TypeId::LookupByName (_protocol);
        _socket = Socket::CreateSocket (GetNode (), tid);
        if (_socket->Bind(_serverAddress) == -1) {
            NS_FATAL_ERROR ("Failed to bind socket");
        }
        _socket->Listen ();
        _socket->ShutdownSend ();

        if(tid == UdpSocketFactory::GetTypeId ()) {
            _socket->SetRecvCallback (MakeCallback (&TraceReplayServer::Recv, this));
        }
        else if(tid == TcpSocketFactory::GetTypeId ()) {
            _socket->SetAcceptCallback( MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
                                         MakeCallback(&TraceReplayServer::HandleTCPAccept, this));
        }

    }
}

void TraceReplayServer::StopApplication(void) {
    NS_LOG_FUNCTION (this);
}

void TraceReplayServer::HandleTCPAccept (Ptr<Socket> socket, const Address& from) {
    _socket = socket;
    _socket->SetRecvCallback (MakeCallback (&TraceReplayServer::Recv, this));
}

void TraceReplayServer::Recv(Ptr<Socket> socket) {
    Ptr<Packet> packet = socket->Recv();
    if(packet->GetSize() == 0) {return;}
}

