#pragma once
#if defined(ARENA_SERVER)
#include <queue>
#include <vector>
#include <common/game_vars.h>
#include <common/GamePackets.h>
#include <common/network_interface.h>
#include <common/types.h>
#include <common/packet.h>
#include <common/mem/memory.h>

#include <common/forward_declare.h>

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, GameHost)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_STRUCT, arena, ClientListener)

struct PlayerInput
{
	glm::vec2 moveDir;
	float jumpTimer;
	// add more when needed;
};

namespace arena
{
    // do not change
    static const int ChallengeHashSize = 1024;

    struct ServerChallengeEntry
    {
        uint64_t m_clientSalt; // generated by client in connection request
        uint64_t m_challengeSalt; // generated by server and sent back to client in challenge pkg
        double m_createdTime; // in seconds, time this challenge was created 
        double m_lastSendTime; // in seconds, the last time challenge packet is sent this client
        ENetPeer* m_peer; // the peer where connection request came from
    };

    struct ServerChallengeHash
    {
        uint32_t m_numEntries;
        uint8_t m_exists[ChallengeHashSize];
        ServerChallengeEntry m_entries[ChallengeHashSize];
        ServerChallengeHash()
        {
            memset(this, 0, sizeof(ServerChallengeHash));
        }
    };

    struct ClientData
    {
        ENetPeer* m_peer;
        uint64_t m_clientSalt;
        uint64_t m_challengeSalt;
        double m_connectTime;
        double m_lastPacketSendTime;
        double m_lastPacketReceiveTime;

        ClientData()
        {
            memset(this, 0, sizeof(ClientData));
        }

		bool operator ==(const ClientData* const lhs) const
		{
			if (lhs == nullptr) return false;

			return ADDRESSOF(lhs) == ADDRESSOF(this);
		}
		bool operator !=(const ClientData* const lhs) const
		{
			return !(lhs == this);
		}
    };

    class Server
    {
    public:
        static const uint32_t MaxClients = 32;

        Server(std::vector<PacketEntry>* sendQueue);
        ~Server();

        void processPacket(Packet* packet, ENetPeer* peer, double timestamp);

        void checkTimeout(double timestamp);

        // adds listener for callbacks
        void addClientListener(ClientListener* listener);
    private:
        std::vector<PacketEntry>* m_sendQueue;

        std::vector<ClientListener*> m_listeners;
        // returns UINT32_MAX if not found
        uint32_t findExistingClientIndex(ENetPeer* host, uint64_t clientSalt, uint64_t challengeSalt) const;

        void receivePackets(double timestamp);

        // process funcs
        void processConnectionRequest(ConnectionRequestPacket* packet, ENetPeer* from, double timestamp);

        void processConnectionResponse(ConnectionResponsePacket* packet, ENetPeer* from, double timestamp);

        void processConnectionKeepAlive(ConnectionKeepAlivePacket* packet, ENetPeer* from, double timestamp);

        void processConnectionDisconnect(ConnectionDisconnectPacket* packet, ENetPeer* from, double timestamp);

        void resetClient(uint32_t clientIndex);

        bool isConnected(uint64_t clientSalt, ENetPeer* peer);

        ServerChallengeEntry* findOrInsertChallenge(ENetPeer* from, uint64_t clientSalt, double timestamp);

        ServerChallengeEntry* findChallenge(ENetPeer* from, uint64_t clientSalt);

        // return UINT32_MAX if no free indices
        uint32_t findFreeClientIndex();

        void connectClient(uint32_t clientIndex, ENetPeer* peer, uint64_t clientSalt, uint64_t challengeSalt, double connectTime);

        void disconnectClient(uint32_t clientIndex, double timestamp);

        void sendPacketToConnectedClient(uint32_t clientIndex, Packet* packet, double timestamp);
    private:
        uint64_t m_serverSalt; // server salt

        ENetPeer* m_clientPeers[MaxClients]; // peers per client
        bool m_clientConnected[MaxClients]; // is client connected
        uint64_t m_clientSalt[MaxClients]; // client salts per client
        uint64_t m_challengeSalt[MaxClients]; // challenge salts per client
        ClientData m_clientData[MaxClients]; // client data per client
        uint32_t m_clientIndices[MaxClients]; // no need to allocate new uint

        uint32_t m_clientsConnected; // number of clients connected
        ServerChallengeHash m_challengeHash; // challenge hashes
    };
}
#endif