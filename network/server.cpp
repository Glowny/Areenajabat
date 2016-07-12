#if defined(ARENA_SERVER)
#include "server.h"
#include <stdio.h>
#include <assert.h>
#include <bx/timer.h>
#include <stdint.h>
#include <common/salt.h>
#include <minini/minIni.h>
#include <fstream>
#include "common/packet.h"
#include <stdio.h>

namespace arena
{
    static const double ChallengeTimeOut = 10.0;
    static const double ChallengeSendRate = 0.1;
    static const double ConnectionConfirmSendRate = 0.1;

    Server::Server() :
        m_clientsConnected(0),
        m_networkInterface(nullptr)
    {
        memset(m_clientPeers, 0, sizeof(m_clientPeers));
        memset(m_clientConnected, 0, sizeof(m_clientConnected));
        memset(m_clientSalt, 0, sizeof(m_clientConnected));
        memset(m_challengeSalt, 0, sizeof(m_challengeSalt));

        m_serverSalt = arena::genSalt();
    }

    Server::~Server()
    {
        delete m_networkInterface;
        m_networkInterface = NULL;
    }
#if 0
    void Server::createPlayerInputs(unsigned playerAmount)
    {
        for (unsigned i = 0; i < playerAmount; i++)
        {
            PlayerInput input;
            input.moveDir.x = 0;
            input.moveDir.y = 0;
            input.jumpTimer = 2;
            m_playerInputVector.push_back(input);
        }
    }
    void Server::createGladiators(unsigned playerAmount)
    {
        m_scoreBoard.flagHolder = 666;

        for (unsigned i = 0; i < playerAmount; i++)
        {
            GladiatorData glad;
            unsigned id = m_physics.addGladiator(glm::vec2(i*100.0f, 50.0f));
            glad.id = id;
            glad.rotation = 0;
            glad.position.x = 100.0f * i;
            glad.position.y = 50.0f;
            glad.hitPoints = 100;
            glad.alive = true;
            m_gladiatorVector.push_back(glad);

            PlayerScore score{ 0,6 };
            m_scoreBoard.PlayerScoreVector.push_back(score);
        }

    }
    void Server::sendSetupPackets(unsigned playerAmount)
    {
        for (unsigned i = 0; i < playerAmount; i++)
        {
            uint32_t size;
            unsigned char* data = createSetupPacket(size, playerAmount, i);
            m_network.sendPacket(data, size, i);
        }
    }

    void Server::sendPlatformPackets()
    {
        uint32_t size2;
        unsigned char* data2 = createPlatformPacket(size2, m_platformVector);
        m_network.broadcastPacket(data2, size2);
    }
#endif
    void Server::receivePackets(double timestamp)
    {
        while (true)
        {
            ENetPeer* peer;
            Packet* packet = m_networkInterface->receivePacket(peer);

            if (packet == nullptr)
            {
                break;
            }

            switch (packet->getType())
            {
            case PacketTypes::ConnectionRequest:
                processConnectionRequest((ConnectionRequestPacket*)packet, peer, timestamp);
                break;
            case PacketTypes::ConnectionResponse:
                processConnectionResponse((ConnectionResponsePacket*)packet, peer, timestamp);
                break;
            case PacketTypes::KeepAlive:
                processConnectionKeepAlive((ConnectionKeepAlivePacket*)packet, peer, timestamp);
                break;
            case PacketTypes::Disconnect:
                processConnectionDisconnect((ConnectionDisconnectPacket*)packet, peer, timestamp);
                break;
            default:
                fprintf(stderr, "Got invalid packet of type %d (not implemented?)", packet->getType());
                break;
            }

            destroyPacket(packet);
        }
    }

    void Server::connectClient(uint32_t clientIndex, ENetPeer* peer, uint64_t clientSalt, uint64_t challengeSalt, double connectTime)
    {
        ARENA_ASSERT(m_clientsConnected < MaxClients - 1, "Server is full");
        ARENA_ASSERT(!m_clientConnected[clientIndex], "Client state is already set to connected");

        ++m_clientsConnected;

        m_clientConnected[clientIndex] = true;

        m_clientData[clientIndex].m_peer = peer;
        m_clientData[clientIndex].m_challengeSalt = challengeSalt;
        m_clientData[clientIndex].m_clientSalt = clientSalt;
        m_clientData[clientIndex].m_connectTime = connectTime;
        m_clientData[clientIndex].m_lastPacketReceiveTime = connectTime;
        m_clientData[clientIndex].m_lastPacketSendTime = connectTime;
        
        m_challengeSalt[clientIndex] = challengeSalt;
        m_clientPeers[clientIndex] = peer;
        m_clientSalt[clientIndex] = clientSalt;

        char buffer[256];
        enet_address_get_host_ip(&peer->address, buffer, sizeof(buffer));
        printf("client %d connected (client address = %s, client salt = %" PRIx64 ", challenge salt = %" PRIx64 ")\n", clientIndex, buffer, clientSalt, challengeSalt);

        ConnectionKeepAlivePacket* ping = (ConnectionKeepAlivePacket*)createPacket(PacketTypes::KeepAlive);
        ping->m_clientSalt = m_clientSalt[clientIndex];
        ping->m_challengeSalt = m_challengeSalt[clientIndex];

        sendPacketToConnectedClient(clientIndex, ping, connectTime);
    }

    void Server::disconnectClient(uint32_t clientIndex, double timestamp)
    {
        ARENA_ASSERT(clientIndex < MaxClients, "clientIndex out of bounds");
        ARENA_ASSERT(m_clientsConnected > 0, "No clients connected");
        ARENA_ASSERT(m_clientConnected[clientIndex], "Client index %d ins't connected", clientIndex);

        ConnectionDisconnectPacket* packet = (ConnectionDisconnectPacket*)createPacket(PacketTypes::Disconnect);
        packet->m_clientSalt = m_clientData[clientIndex].m_clientSalt;
        packet->m_challengeSalt = m_clientData[clientIndex].m_challengeSalt;

        sendPacketToConnectedClient(clientIndex, packet, timestamp);

        resetClient(clientIndex);
        --m_clientsConnected;
    }

    void Server::sendPacketToConnectedClient(uint32_t clientIndex, Packet* packet, double timestamp)
    {
        ARENA_ASSERT(clientIndex < MaxClients, "Invalid client index");
        ARENA_ASSERT(m_clientConnected[clientIndex], "Client isn't connected");
        ARENA_ASSERT(packet != nullptr, "Packet can not be NULL");
        ARENA_ASSERT(m_clientPeers[clientIndex] != nullptr, "Peer can not be NULL");
        m_networkInterface->sendPacket(m_clientPeers[clientIndex], packet);
        m_clientData[clientIndex].m_lastPacketSendTime = timestamp;
    }

    uint32_t Server::findExistingClientIndex(ENetPeer* host, uint64_t clientSalt, uint64_t challengeSalt) const
    {
        enet_uint32 address = host->address.host;
        for (uint32_t i = 0; i < MaxClients; ++i)
        {
            if (m_clientConnected[i] &&
                m_clientPeers[i]->address.host == address
                && m_clientSalt[i] == clientSalt
                && m_challengeSalt[i] == challengeSalt)
            {
                return i;
            }
        }
        return UINT32_MAX;
    }

    uint32_t Server::findFreeClientIndex()
    {
        for (uint32_t i = 0; i < MaxClients; ++i)
        {
            if (!m_clientConnected[i]) return i;
        }

        return UINT32_MAX;
    }

    void Server::processConnectionRequest(ConnectionRequestPacket* packet, ENetPeer* from, double timestamp)
    {
        BX_UNUSED(packet, timestamp);
        char buffer[256];
        enet_address_get_host_ip(&from->address, buffer, sizeof(buffer));
        printf("GOt connection request packet from: %s\n", buffer);

        if (m_clientsConnected == MaxClients)
        {
            printf("Server is full\n");
            ConnectionDeniedPacket* denied = (ConnectionDeniedPacket*)createPacket(PacketTypes::ConnectionDenied);
            denied->m_clientSalt = packet->m_clientSalt;
            denied->m_reason = ConnectionDeniedPacket::ConnectionDeniedReason::ServerIsFull;
            m_networkInterface->sendPacket(from, denied);
        }

        else if (isConnected(packet->m_clientSalt, from))
        {
            printf("Client (%" PRIu64 ") is already connected!\n", packet->m_clientSalt);

            ConnectionDeniedPacket* denied = (ConnectionDeniedPacket*)createPacket(PacketTypes::ConnectionDenied);
            denied->m_clientSalt = packet->m_clientSalt;
            denied->m_reason = ConnectionDeniedPacket::ConnectionDeniedReason::AlreadyConnected;

            m_networkInterface->sendPacket(from, denied);
        }

        ServerChallengeEntry* entry = findOrInsertChallenge(from, packet->m_clientSalt, timestamp);
        
        if (entry == nullptr)
        {
            return;
        }

        ARENA_ASSERT(entry->m_peer->address.host == from->address.host, "Host mismatch");
        ARENA_ASSERT(entry->m_clientSalt == packet->m_clientSalt, "Client salt mismatch");

        if (entry->m_lastSendTime + ChallengeSendRate < timestamp)
        {
            printf("Sending connection challenge to %s (challenge salt = %" PRIx64 ")\n", buffer, entry->m_challengeSalt);
            ConnectionChallengePacket* packet = (ConnectionChallengePacket*)createPacket(PacketTypes::ConnectionChallenge);
            packet->m_clientSalt = entry->m_clientSalt;
            packet->m_challengeSalt = entry->m_challengeSalt;
            
            m_networkInterface->sendPacket(from, packet);

            entry->m_lastSendTime = timestamp;
        }
    }

    void Server::processConnectionResponse(ConnectionResponsePacket* packet, ENetPeer* from, double timestamp)
    {
        uint32_t idx = findExistingClientIndex(from, packet->m_clientSalt, packet->m_challengeSalt);

        // if it exists
        if (idx != UINT32_MAX)
        {
            ARENA_ASSERT(idx < MaxClients, "Invalid client id (%" PRIu32 ") max (%" PRIu32 ")", idx, MaxClients);

            if (m_clientData[idx].m_lastPacketSendTime + ConnectionConfirmSendRate < timestamp)
            {
                ConnectionKeepAlivePacket* ping = (ConnectionKeepAlivePacket*)createPacket(PacketTypes::KeepAlive);
                ping->m_clientSalt = packet->m_clientSalt;
                ping->m_challengeSalt = packet->m_challengeSalt;
                sendPacketToConnectedClient(idx, ping, timestamp);
            }
        }
        else
        {
            char buf[256];
            enet_address_get_host_ip(&from->address, buf, sizeof(buf));
            printf("processing connection response from client %s (salt %" PRIx64 ") (challenge %" PRIx64 ")\n", buf, packet->m_clientSalt, packet->m_challengeSalt);

            ServerChallengeEntry* entry = findChallenge(from, packet->m_clientSalt);

            if (entry == nullptr) return;

            ARENA_ASSERT(entry->m_clientSalt == packet->m_clientSalt, "Client salt mismatch");

            if (entry->m_challengeSalt != packet->m_challengeSalt)
            {
                printf("conneciton challenge mismatch expected %" PRIx64 ", got %" PRIx64 "\n", entry->m_challengeSalt, packet->m_challengeSalt);
                return;
            }

            if (m_clientsConnected == MaxClients)
            {
                // TODO send denied
                ARENA_ASSERT(0, "serverf is full");
            }

            const uint32_t clientIndex = findFreeClientIndex();

            ARENA_ASSERT(clientIndex != UINT32_MAX, "Invalid client index");

            connectClient(clientIndex, from, packet->m_clientSalt, packet->m_challengeSalt, timestamp);
        }
    }

    void Server::processConnectionKeepAlive(ConnectionKeepAlivePacket* packet, ENetPeer* from, double time)
    {
        const uint32_t idx = findExistingClientIndex(from, packet->m_clientSalt, packet->m_challengeSalt);
        // does not exist
        if (idx == UINT32_MAX) return;

        ARENA_ASSERT(idx < MaxClients, "Client id out of bounds");

        m_clientData[idx].m_lastPacketReceiveTime = time;
    }

    void Server::processConnectionDisconnect(ConnectionDisconnectPacket* packet, ENetPeer* from, double timestamp)
    {
        const uint32_t idx = findExistingClientIndex(from, packet->m_clientSalt, packet->m_challengeSalt);

        if (idx == UINT32_MAX) return;

        ARENA_ASSERT(idx < MaxClients, "Client id out of bounds");

        disconnectClient(idx, timestamp);
    }

    ServerChallengeEntry* Server::findOrInsertChallenge(ENetPeer* from, uint64_t clientSalt, double timestamp)
    {
        const uint64_t key = calculateChallengeHash(from, clientSalt, m_serverSalt);

        uint32_t index = key % ChallengeHashSize;

#if 0
        printf("Client salt    = %" PRIx64 "\n", clientSalt);
        printf("Challenge hash = %" PRIx64 "\n", key);
        printf("Challenge idx  = %" PRIu32 "\n", index);
#endif

        if (!m_challengeHash.m_exists[index] || (m_challengeHash.m_exists[index] && m_challengeHash.m_entries[index].m_createdTime + ChallengeTimeOut < timestamp))
        {
            printf("found empty entry in challenge hash (idx = %" PRIu32 ")\n", index);
            ServerChallengeEntry* entry = &m_challengeHash.m_entries[index];
            entry->m_clientSalt = clientSalt;
            entry->m_challengeSalt = genSalt();
            // force send
            entry->m_createdTime = timestamp;
            entry->m_lastSendTime = timestamp - (2.0 * ChallengeSendRate); // hmm
            entry->m_peer = from;

            m_challengeHash.m_exists[index] = 1;

            return entry;
        }

        if (m_challengeHash.m_exists[index]
            && m_challengeHash.m_entries[index].m_clientSalt == clientSalt
            && m_challengeHash.m_entries[index].m_peer->address.host == from->address.host)
        {
            printf("found existing challenge hash (idx = %" PRIu32 ")\n", index);
            return &m_challengeHash.m_entries[index];
        }
        return nullptr;
    }

    ServerChallengeEntry* Server::findChallenge(ENetPeer* from, uint64_t clientSalt)
    {
        const uint64_t key = calculateChallengeHash(from, clientSalt, m_serverSalt);

        uint32_t index = key % ChallengeHashSize;

#if 0
        printf("Client salt    = %" PRIx64 "\n", clientSalt);
        printf("Challenge hash = %" PRIx64 "\n", key);
        printf("Challenge idx  = %" PRIu32 "\n", index);
#endif

        if (m_challengeHash.m_exists[index]
            && m_challengeHash.m_entries[index].m_clientSalt == clientSalt
            && m_challengeHash.m_entries[index].m_peer->address.host == from->address.host)
        {
            printf("found existing challenge hash (idx = %" PRIu32 ")\n", index);
            return &m_challengeHash.m_entries[index];
        }
        return nullptr;
    }

    void Server::resetClient(uint32_t clientIndex)
    {
        ARENA_ASSERT(clientIndex < MaxClients, "Client index out of bounds");
        m_clientConnected[clientIndex] = false;
        m_clientSalt[clientIndex] = 0;
        m_challengeSalt[clientIndex] = 0;
        m_clientPeers[clientIndex] = 0;
        m_clientData[clientIndex] = ClientData();
    }

    bool Server::isConnected(uint64_t clientSalt, ENetPeer* peer)
    {
        ARENA_ASSERT(peer != nullptr, "Peer can not be nullptr");
        ARENA_ASSERT(clientSalt != 0, "Invalid client salt");

        for (uint32_t i = 0; i < MaxClients; ++i)
        {
            if (!m_clientConnected[i]) continue;

            if (m_clientSalt[i] == clientSalt && m_clientPeers[i]->address.host == peer->address.host)
                return true;
        }
        return false;
    }

    void Server::start(uint16_t port, unsigned playerAmount)
    {
        (void)port;
        (void)playerAmount;
        
        //loadPlatformsFromFile("coordinatesRawData.dat");
        //m_messageQueue = new std::queue<Message>;
        //m_network.startServer(m_messageQueue, 0, port, 10);

        // wait for players..

        int64_t lastTime = bx::getHPCounter();
        double totalTime = 0.0;
        while (true)
        {
            // More time stuff
            int64_t currentTime = bx::getHPCounter();
            const int64_t time = currentTime - lastTime;
            lastTime = currentTime;
            const double frequency = (double)bx::getHPFrequency();
            // seconds
            float lastDeltaTime = float(time * (1.0f / frequency));
            totalTime += lastDeltaTime;

            m_networkInterface->writePackets();
            
            // this call will fill receive queue
            m_networkInterface->readPackets();

            receivePackets(totalTime);
            
#if 0
            while (m_network.getConnectedPlayerAmount() < m_playerAmount)
            {
                m_network.checkEvent();
                // Might be dangerous if client client send bad messages, add checks.
                handleClientMessages();
            }
            sendPlatformPackets(); //TOOD: send only if needed
            std::vector<unsigned> idVector;

            printf("GAME STARTING!\n");

            createPlayerInputs(m_playerAmount);
            createGladiators(m_playerAmount);
            for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
            {
                idVector.push_back(i);
            }
            m_network.setPlayerIds(idVector);

            // Allocate memory for update messages that are send on every game loop.
            m_updateSize = sizeof(MessageIdentifier) + sizeof(float) * 5 * m_gladiatorVector.size();
            m_updateMemory = (unsigned char*)malloc(m_updateSize);

            sendSetupPackets(m_playerAmount);

            // Time stuff
            int64_t s_last_time = bx::getHPCounter();
            float updatePhysics = 0;
            float updateNetwork = 0;
            float respawnPlayer = 0;
            float updateScore = 0;

            float timeStep = 1.0f / 60.0f;
            m_run = true;

            while (m_run)
            {
                // More time stuff
                int64_t currentTime = bx::getHPCounter();
                const int64_t time = currentTime - s_last_time;
                s_last_time = currentTime;
                const double frequency = (double)bx::getHPFrequency();
                // seconds
                float lastDeltaTime = float(time * (1.0f / frequency));
                updatePhysics += lastDeltaTime;
                updateNetwork += lastDeltaTime;
                respawnPlayer += lastDeltaTime;
                updateScore += lastDeltaTime;
                // No more time stuff

                for (unsigned i = 0; i < m_playerInputVector.size(); i++)
                {
                    m_playerInputVector[i].jumpTimer += lastDeltaTime;
                }

                if (updatePhysics > timeStep)
                {
                    // Update movement and physics.
                    gladiatorMovement();
                    m_physics.update();

                    // Get latest positions and velocities.
                    for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
                    {
                        m_gladiatorVector[i].position = m_physics.getGladiatorPosition(i);
                        m_gladiatorVector[i].velocity = m_physics.getGladiatorVelocity(i);
                    }

                    updatePhysics = 0;
                }
                m_network.checkEvent();

                handleClientMessages();

                sendBulletCreationEvents();
                sendBulletHitEvents();

                if (respawnPlayer > 5.0f)
                {
                    respawnDeadPlayers();
                    respawnPlayer = 0;
                }

                if (updateNetwork > 0.1)
                {
                    sendGameUpdateMessages();
                    updateNetwork = 0;
                }
                if (updateScore > 1)
                {
                    uint32_t size;
                    unsigned char* data = createScoreboardUpdatePacket(size, m_scoreBoard);
                    m_network.broadcastPacket(data, size, false);
                    updateScore = 0;
                }

            }
            printf("Server restarting... \n");
            m_gladiatorVector.clear();
            m_playerInputVector.clear();
            m_physics.reset();
            m_scoreBoard.flagHolder = 666;
            m_scoreBoard.PlayerScoreVector.clear();

            printf("Restart complete, waiting for players.. \n");
#endif
        }
    }

    void Server::start(const String& iniPath)
    {
        const minIni Ini(iniPath);
        const String ServerSection = "server";
        const String GamemodeSection = "gamemode";

        // todo this isnt needed
        //const uint32 address		= uint32(Ini.geti(ServerSection, "address", 0));
        const uint32 port = uint32(Ini.geti(ServerSection, "port", 8888));
        const uint32 maxPlayers = uint32(Ini.geti(GamemodeSection, "max_players", 4));

        m_networkInterface = new arena::NetworkInterface(uint16_t(port));

        start(port, maxPlayers);
    }
#if 0
    void Server::sendGameUpdateMessages()
    {
        unsigned char *data = createUpdatePacket(m_updateSize, m_gladiatorVector, m_updateMemory);
        m_network.broadcastPacket(data, m_updateSize, false);

        if (m_physics.m_bulletVector.size() != 0)
        {
            std::vector<glm::vec2> updateBulletPositions;
            std::vector<glm::vec2> updateBulletVelocities;
            for (unsigned i = 0; i < m_physics.m_bulletVector.size(); i++)
            {
                b2Vec2 position = m_physics.m_bulletVector[i]->m_body->GetPosition();
                b2Vec2 velocity = m_physics.m_bulletVector[i]->m_body->GetLinearVelocity();
                updateBulletPositions.push_back(glm::vec2(position.x * 100, position.y * 100));
                updateBulletVelocities.push_back(glm::vec2(velocity.x, velocity.y));
            }
            uint32_t bulletUpdateSize;
            unsigned char *data2 = createBulletUpdatePacket(bulletUpdateSize, updateBulletPositions, updateBulletVelocities);
            m_network.broadcastPacket(data2, bulletUpdateSize, false);

        }
    }

    void Server::respawnDeadPlayers()
    {
        printf("respawned\n");
        for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
        {
            if (m_gladiatorVector[i].alive == false)
            {
                m_gladiatorVector[i].alive = true;
                m_gladiatorVector[i].hitPoints = 100;
                uint32_t size;
                unsigned char* data = createPlayerRespawnPacket(size, i);
                m_network.broadcastPacket(data, size, true);
                m_physics.setGladiatorPosition(i, glm::vec2(2000, 50));
            }
        }
    }

    void Server::sendBulletHitEvents()
    {
        if (m_physics.hitVector.size() != 0)
        {
            for (unsigned i = 0; i < m_physics.hitVector.size(); i++)
            {
                switch (m_physics.hitVector[i].hitType)
                {
                case B_Platform:
                {
                    uint32_t size;
                    glm::vec2 position;
                    position.x = m_physics.hitVector[i].position.x * 100;
                    position.y = m_physics.hitVector[i].position.y * 100;
                    unsigned char *data = createHitPacket(size, position);
                    m_network.broadcastPacket(data, size, true);
                    break;
                }
                case B_Gladiator:
                {
                    uint32_t size;
                    glm::vec2 position;
                    unsigned playerId = m_physics.hitVector[i].targetPlayerId;
                    position.x = m_physics.hitVector[i].position.x * 100;
                    position.y = m_physics.hitVector[i].position.y * 100;
                    unsigned char *data = createHitPacket(size, position);
                    m_network.broadcastPacket(data, size, true);
                    data = createPlayerDamagePacket(size, playerId, 10);
                    m_gladiatorVector[playerId].hitPoints -= 10;
                    m_network.broadcastPacket(data, size, true);
                    // unsigned, change to signed or do some magic tricts
                    if (m_gladiatorVector[playerId].hitPoints == 0)
                    {
                        data = createPlayerKillPacket(size, playerId);
                        m_network.broadcastPacket(data, size, true);
                        m_gladiatorVector[playerId].alive = false;
                        m_scoreBoard.PlayerScoreVector[playerId].tickets--;
                        unsigned killerID = m_physics.hitVector[i].shooterPlayerId;
                        m_scoreBoard.PlayerScoreVector[killerID].score++;
                    }
                }
                default:
                    break;
                }
            }
            m_physics.hitVector.clear();
        }
    }

    void Server::sendBulletCreationEvents()
    {
        // Create send bullet creation events for client.
        if (m_bulletOutputVector.size() != 0)
        {
            uint32_t size;
            unsigned char *data = createBulletCreationPacket(size, m_bulletOutputVector);
            m_network.broadcastPacket(data, size, true);
            m_bulletOutputVector.clear();
        }
    }

    void Server::gladiatorMovement()
    {
        for (unsigned i = 0; i < m_playerInputVector.size(); i++)
        {
            if (m_gladiatorVector[i].alive == true)
            {

                if (m_playerInputVector[i].moveDir.y != 0 && m_playerInputVector[i].jumpTimer > 1)
                {
                    m_physics.ApplyImpulseToGladiator(glm::vec2(m_playerInputVector[i].moveDir.x * 100, m_playerInputVector[i].moveDir.y * 400), i);
                    m_playerInputVector[i].moveDir.y = 0.0;
                    m_playerInputVector[i].jumpTimer = 0;
                }
                if (m_playerInputVector[i].moveDir.x != 0)
                {
                    float xVelocity = m_physics.getGladiatorVelocity(i).x;
                    if (xVelocity < 250 && xVelocity > -250)
                        m_physics.AppleForceToGladiator(glm::vec2(m_playerInputVector[i].moveDir.x * 1500, 0), i);
                    m_playerInputVector[i].moveDir.x = 0.0;

                }

            }
        }
    }

    void Server::handleClientMessages()
    {
        while (m_messageQueue->size() != 0)
        {
            handleMessage(m_messageQueue->front());
            free(m_messageQueue->front().data);
            m_messageQueue->pop();
        }
    }
    void Server::handleMessage(Message &message)
    {
        //TODO: Check that messages are legit
        unsigned playerId = message.playerID;
        MessageIdentifier packetID = getMessageID(message.data);

        switch (packetID)
        {
        case ClientMove:
            openMovePacket(message.data, m_playerInputVector[playerId].moveDir);
            break;
        case ClientShoot:
        {
            // These are bullet creation events.
            std::vector<BulletInputData> bulletInputVector;
            openBulletRequestPacket(message.data, bulletInputVector);
            createOutputBullets(bulletInputVector, playerId);
            break;
        }
        case Restart:
            m_run = false;
            break;
        case PlayerAmount:
        {
            if (!m_run)
            {
                openPlayerAmountPacket(message.data, m_playerAmount);
            }
            break;
        }

        default:
            break;
        }
    }

    glm::vec2 radToVec(float r)
    {
        return glm::vec2(cos(r), sin(r));
    }

    void Server::createOutputBullets(std::vector<BulletInputData> &bulletInputVector, unsigned playerId)
    {
        for (unsigned i = 0; i < bulletInputVector.size(); i++)
        {
            BulletOutputData bullet;
            bullet.bulletType = bulletInputVector[i].bulletType;
            bullet.playerId = playerId;

            if (m_gladiatorVector[bullet.playerId].alive)
            {
                switch (bullet.bulletType)
                {
                case UMP45:
                {
                    glm::vec2 vectorAngle = radToVec(bulletInputVector[i].rotation);
                    bullet.position.x = m_gladiatorVector[playerId].position.x + vectorAngle.x * 70;
                    bullet.position.y = m_gladiatorVector[playerId].position.y - 32 + vectorAngle.y * 70;
                    bullet.rotation = bulletInputVector[i].rotation;

                    bullet.velocity.x = vectorAngle.x * 10;
                    bullet.velocity.y = vectorAngle.y * 10;

                    m_physics.addBullet(bullet.position, bullet.velocity, bullet.playerId);
                }
                case Shotgun:
                {

                }
                }
                m_bulletOutputVector.push_back(bullet);
            }
        }

    }

    struct platformObject
    {
        unsigned type;
        uint32_t size;
        std::vector<glm::vec2> points;
    };

    void Server::loadPlatformsFromFile(char* filename)
    {
        std::ifstream file;

        file.open(filename, std::ios::in | std::ios::binary);

        if (!file.is_open())
            printf("FILE NOT FOUND %s", filename);
        uint32_t sizes;
        file.read(reinterpret_cast<char*>(&sizes), sizeof(uint32_t));

        std::vector<platformObject> objects;
        for (unsigned i = 0; i < sizes; i++)
        {
            platformObject object;
            file.read(reinterpret_cast<char*>(&object.size), sizeof(uint32_t));
            file.read(reinterpret_cast<char*>(&object.type), sizeof(unsigned int));


            for (unsigned i = 0; i < object.size; i++)
            {
                float x;
                file.read(reinterpret_cast<char*>(&x), sizeof(float));
                object.points.push_back(glm::vec2(x, 0));
            }
            for (unsigned i = 0; i < object.size; i++)
            {
                float y;
                file.read(reinterpret_cast<char*>(&y), sizeof(float));
                object.points[i].y = y;
            }
            objects.push_back(object);
        }
        file.close();

        for (unsigned i = 0; i < objects.size(); i++)
        {
            Platform platform;
            for (unsigned j = 0; j < objects[i].points.size(); j++)
            {
                platform.points.push_back(objects[i].points[j]);
            }
            m_physics.createPlatform(platform.points);
            m_platformVector.push_back(platform);
        }

    }
#endif
}
#endif