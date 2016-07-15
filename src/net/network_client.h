#pragma once

#include <stdint.h>
#include <enet/enet.h>
#include <common/network_interface.h>
#include <bx/bx.h>

namespace arena
{
    struct ClientState
    {
        enum Enum
        {
            Disconnected,
            Connected,
            SendingAuthResponse,
            SendingConnectionRequest,
            Count,
        };
    };

    struct LobbyState
    {
        enum Enum
        {
            NotInLobby,
            SendingCreateLobby,
            SendingJoinLobby,
            InLobby,
            SendingQueryLobbies,
            Count
        };
    };

    struct BX_NO_VTABLE LobbyListener
    {
        virtual ~LobbyListener() = 0;
        virtual void onLobbyList(class NetworkClient* sender, struct LobbyQueryResultPacket* response, double timestamp) = 0;
        virtual void onLobbyCreationResult(class NetworkClient* sender, struct LobbyResultPacket* response, double timestamp) = 0;
    };

    inline LobbyListener::~LobbyListener() {}

    struct Lobby
    {
        std::string name;
    };

    class NetworkClient
    {
    public:
        NetworkClient(uint16_t clientPort);

        void connect(const char* address, uint16_t port, double timestamp);

        void disconnect(double timestamp);

        bool isConnected() const;

        bool isConnecting() const;

        // send pings disconnects and connects
        void sendProtocolPackets(double timestamp);

        void writePackets();

        void readPackets();

        Packet* receivePacket(ENetPeer*& from);

        // process pings disconnects and connects
        void processClientSidePackets(Packet* packet, ENetPeer* from, double timestamp);

        void processMatchmakingPackets(Packet* packet, ENetPeer* from, double timestamp);

        void sendPacketToServer(Packet* packet, double timestamp);

        void createLobby(const char* name, double timestamp);

        void queryLobbies(double timestamp);

        void sendMatchMakingPackets(double timestamp);

        LobbyListener* m_lobbyListener;

        void requestJoinLobby(uint64_t lobbySalt, double timestamp);
    private:
        void reset();

        NetworkInterface m_networkInterface;

        ENetPeer* m_peer;

        ClientState::Enum m_state;
        LobbyState::Enum m_lobbyState;


        ENetAddress m_serverAddress;
        // time we last sent a packet
        double m_lastPacketSendTime;

        // time we last received a packet
        double m_lastPacketReceivedTime;

        // client salt for challenge auth
        uint64_t m_clientSalt;

        // got from server after auth
        uint64_t m_challengeSalt;

        Lobby m_currentLobby;
    };
}