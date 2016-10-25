#include "sandbox_scene.h"
#include "../app.h"
#include "../game_time.h"

#include "../ecs/projectile.h"
#include "../ecs/timer.h"
#include "../ecs/movement.h"
#include "../ecs/transform.h"
#include "../ecs/animator.h"
#include "../ecs/sprite_renderer.h"
#include "../ecs/entity.h"
#include "../ecs/entity_builder.h"
#include "../ecs/managers/sprite_manager.h"
#include "../ecs/managers/animator_manager.h"
#include "../ecs/managers/projectile_manager.h"
#include "../ecs/managers/physics_manager.h"
#include "../ecs/managers/transform_manager.h"
#include "../ecs/managers/trail_manager.h"
#include "../ecs/managers/character_manager.h"
#include "../ecs/managers/camera_manager.h"
#include "../ecs/bullet_trail.h"
#include "../ecs/player_link.h"

#include "../res/resource_manager.h"
#include "../res/texture_resource.h"
#include "../res/spriter_resource.h"

#include "../net/network_client.h"

#include "../graphics/character_animator.h"
#include "../graphics/spritebatch.h"

#include "../utils/color.h"
#include "../utils/math.h"

#include "../input/event.h"

#include <common/arena/gladiator.h>
#include <common/packet.h>
#include <bx/fpumath.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb/stb_truetype.h"
#ifdef _DEBUG

#   include "../rtti/rtti_define.h"
#endif


namespace arena
{
	glm::vec2 radToVec(float r)
	{
		return glm::vec2(cos(r), sin(r));
	}

	void loadServerInfoFromFile(std::string& ip, uint16_t& port)
	{
		std::ifstream file("assets/ServerInfo.txt");
		if (file.is_open())
		{
			getline(file, ip);
			std::string tempString;
			getline(file, tempString);
			port = (uint16)std::stoul(tempString, nullptr, 0);

		}
	}

	SandboxScene* sandbox;
	static NetworkClient* s_client;
	static double s_stamp = 0.0;
	struct DebugLobbyListener : public LobbyListener
	{
		~DebugLobbyListener() override {}

		virtual void onLobbyList(NetworkClient* sender, LobbyQueryResultPacket* response, double timestamp) override
		{
			(void)sender; (void)timestamp;
			fprintf(stderr, "Lobby count: %d\n", response->m_lobbyCount);
			for (int32_t i = 0; i < response->m_lobbyCount; ++i)
			{
				fprintf(stderr, "\t%s, uid=%" PRIx64 "\n", response->m_lobbynames[i], response->m_lobbySalt[i]);
			}

			if (response->m_lobbyCount == 0)
			{
				static const char s_debugLobbyName[] = "perkele";
				fprintf(stderr, "Requesting to create lobby \"%s\"\n", s_debugLobbyName);
				sender->requestCreateLobby(s_debugLobbyName, s_stamp);
			}
			else
			{
				fprintf(stderr, "Trying to join lobby %" PRIx64 "\n", response->m_lobbySalt[0]);
				// as debug join the first one
				sender->requestJoinLobby(response->m_lobbySalt[0], timestamp);
			}
		}

		virtual void onLobbyCreationResult(NetworkClient* sender, LobbyResultPacket* response, double timestamp) override
		{
			BX_UNUSED(sender, response, timestamp);
			if (response->m_created)
			{
				fprintf(stderr, "Created lobby (salt = %" PRIx64 ")\n", response->m_lobbySalt);

				fprintf(stderr, "Trying to join lobby %" PRIx64 "\n", response->m_lobbySalt);

				sender->requestJoinLobby(response->m_lobbySalt, timestamp);
			}
			else
			{
				// TODO reason
				fprintf(stderr, "Failed to create lobby\n");
			}
		}

		virtual void onLobbyJoinResult(NetworkClient* sender, LobbyJoinResultPacket* response, double timestamp) override
		{
			BX_UNUSED(sender, response, timestamp);

			if (response->m_joined)
			{
				fprintf(stderr, "Joined lobby\n");
				sandbox->gameRunning = true;
			}
			else
			{
				fprintf(stderr, "Failed to join lobby: %s\n",
					(response->m_reason == LobbyJoinResultPacket::LobbyDoesNotExist)
					? "lobby does not exist" : "already joined");
			}
		}
	};
	static DebugLobbyListener s_lobbyListener;


	static void connect(const void*)
	{
		if (s_client->isConnected()) return;
		if (s_client->isConnecting()) return;
		std::string ip = "localhost";
		uint16_t port = 8088;
		loadServerInfoFromFile(ip, port);
		s_client->connect(ip.c_str(), port, s_stamp);
		s_client->queryLobbies(s_stamp);
	}

	static void disconnect(const void*)
	{
		if (!s_client->isConnected()) return;

		s_client->disconnect(s_stamp);
	}

	static const InputBinding s_bindings[] =
	{
		{ arena::Key::KeyA, arena::Modifier::None, 0, inputMoveLeft, "moveleft" },
		{ arena::Key::KeyD, arena::Modifier::None, 0, inputMoveRight, "moveright" },
		{ arena::Key::KeyW, arena::Modifier::None, 0, inputMoveUp, "moveup" },
		{ arena::Key::KeyS, arena::Modifier::None, 0, inputMoveDown, "movedown" },
		{ arena::Key::KeyQ, arena::Modifier::None, 0, connect, "connect" },
		{ arena::Key::Key9, arena::Modifier::None, 0, disconnect, "disconnect" },
		{ arena::Key::KeyR, arena::Modifier::None, 0, inputReload, "reload"},
		{ arena::Key::Space, arena::Modifier::None, 0, inputJump, "jump" },
		{ arena::Key::F1, arena::Modifier::None, 0, toggleKeyBindDraw, "toggleKeyBindDraw" },
		{ arena::Key::Tab, arena::Modifier::None, 0, toggleScoreBoardDraw, "toggleScoreBoardDraw" },
		{ arena::Key::Plus, arena::Modifier::None, 0, morePlayers, "morePlayers" },
		{ arena::Key::Minus, arena::Modifier::None, 0, lessPlayers, "lessPlayers" },
		INPUT_BINDING_END
	};


	static void inputMoveLeft(const void*)
	{
		sandbox->m_controller.m_input.m_leftButtonDown = true;
	}
	static void inputMoveRight(const void*)
	{
		sandbox->m_controller.m_input.m_rightButtonDown = true;
	}
	static void inputMoveUp(const void*)
	{
		sandbox->m_controller.m_input.m_upButtonDown = true;
	}
	static void inputMoveDown(const void*)
	{
		sandbox->m_controller.m_input.m_downButtonDown = true;
	}

	static void inputReload(const void*)
	{
		sandbox->m_controller.m_input.m_reloadButtonDown = true;
	}
	static void inputJump(const void*)
	{
		sandbox->m_controller.m_input.m_jumpButtonDown = true;
	}

	static void toggleKeyBindDraw(const void*)
	{
		sandbox->m_toggleKeyBindDraw = !sandbox->m_toggleKeyBindDraw;
	}
	static void toggleScoreBoardDraw(const void*)
	{
		sandbox->m_toggleScoreboardDraw = !sandbox->m_toggleScoreboardDraw;
	}
	static void morePlayers(const void*)
	{
		sandbox->m_playerChange = 1;
	}
	static void lessPlayers(const void*)
	{
		sandbox->m_playerChange = -1;
	}

	SandboxScene::SandboxScene() : Scene("sandbox")
	{
		gameRunning = false;
		hasMap = false;
		// Pointer to scene for input to use.
		sandbox = this;
		m_sendInputToServerTimer = 0;
		m_controller.aimAngle = 0;
		m_factory = new EntityFactory(&m_physics, this);
		CharacterManager::instance().setFactory(m_factory);
		PhysicsManager::instance().setPhysics(&m_physics);
		createBackground();
		m_scoreboard = nullptr;
		m_gameMode = nullptr;
		m_toggleKeyBindDraw = true;
		
	}

	void SandboxScene::onUpdate(const GameTime& gameTime)
	{
		s_stamp = gameTime.m_total;
		MouseState state = Mouse::getState();
		if (state.m_buttons[MouseButton::Left])
		{
			m_controller.m_input.m_shootButtonDown = true;
		}
		if (state.m_buttons[MouseButton::Right])
		{
			m_controller.m_input.m_grenadeButtonDown = true;
		}
		if (!s_client->isConnected())
		{
			if (gameRunning)
			{
				gameRunning = false;
			}
			else
				cleanUp();
		}
		// Send packets related to finding match.
		s_client->sendMatchMakingPackets(gameTime.m_total);
		// Send packets related to connection upkeep.
		s_client->sendProtocolPackets(gameTime.m_total);

		// Send player input to server if 1/60 of a second has passed.
		m_physicsUpdated = false;
		if ((m_physics.updateTimer += static_cast<float32>(gameTime.m_delta)) >= PHYSICS_TIMESTEP && s_client->isConnected())
		{
			updatePhysics(m_physics.updateTimer);
			m_physicsUpdated = true;
			m_physics.updateTimer = 0;
		}

		if ((m_sendInputToServerTimer += gameTime.m_delta) >= 0.016f && s_client->isConnected())
		{
			sendInput(m_controller);
			m_sendInputToServerTimer = 0;;
			
		}
		// Get packets form network.
		s_client->readPackets();
		// Process packets stored in s_client.
		processAllPackets(gameTime);
		// Write current packets to network.
		s_client->writePackets();

		//TEMP: if game has not started, do not update. Create some other way to check if a round is running!
		if (CharacterManager::instance().getCharacterAmount() != 0)
		{
			updateEntities(gameTime);
			Transform* mouseTransform = (Transform* const)CameraManager::instance().m_mousePointer->first(TYPEOF(Transform));
			m_controller.aimAngle = CharacterManager::instance().rotateCharacterAnimationAim(m_playerId, mouseTransform->m_position);
			CharacterManager::instance().onUpdate(gameTime);
			CameraManager::instance().setMouseState(Mouse::getState());
			CameraManager::instance().onUpdate(gameTime);
		}
		// While in lobby, send player amount change requests every second if needed.
		// This is unsecure on server side atm.
		else
		{
			m_changeTimer += gameTime.m_delta;
			if (m_changeTimer > 1.0f && s_client->isConnected())
			{ 
				m_changeTimer = 0;
				if (m_playerChange != 0)
				{
					m_playerAmount += m_playerChange;
					if (m_playerAmount < 1)
						m_playerAmount = 1;
					else if (m_playerAmount > 8)
						m_playerAmount = 8;
					m_playerChange = 0;
					
					GameSetPlayerAmountPacket* packet = (GameSetPlayerAmountPacket*)createPacket(PacketTypes::GameSetPlayerAmount);
					packet->m_playerAmount = m_playerAmount;
					s_client->sendPacketToServer(packet, s_stamp);
					printf("Setting player amount to %i, sending message to the server.\n", m_playerAmount);
				}
			}
		}
		
		SpriteManager::instance().update(gameTime);
		AnimatorManager::instance().update(gameTime);
		TrailManager::instance().update(gameTime);
		App::instance().spriteBatch()->submit(0);

		// Set current debug draw text.
		setDrawText(gameTime);
	}
	void SandboxScene::onInitialize()
	{
		srand((uint32_t)time(NULL));
		s_client = new NetworkClient();
		s_client->m_lobbyListener = &s_lobbyListener;
		m_playerId = 0;

		inputAddBindings("player", s_bindings);
		// set the object camera follows.
		CameraManager::instance().m_mousePointer = m_factory->createMousePointer(App::instance().camera());
	}
	void SandboxScene::onDestroy()
	{
		inputRemoveBindings("player");
	}

	void SandboxScene::sendInput(PlayerController &controller)
	{
		GameInputPacket* packet = (GameInputPacket*)createPacket(PacketTypes::GameInput);
		packet->m_aimAngle = controller.aimAngle;
		packet->m_input = controller.m_input;
		packet->m_clientSalt = s_client->m_clientSalt;
		packet->m_challengeSalt = s_client->m_challengeSalt;
		s_client->sendPacketToServer(packet, s_stamp);
		memset(&m_controller.m_input, false, sizeof(PlayerInput));
	}
	void SandboxScene::requestMap(uint8_t mapID)
	{
		if (hasMap)
			return;
		hasMap = true;
		GameRequestMapPacket* packet = (GameRequestMapPacket*)createPacket(PacketTypes::GameRequestMap);
		packet->mapID = mapID;

		packet->m_clientSalt = s_client->m_clientSalt;
		packet->m_challengeSalt = s_client->m_challengeSalt;

		s_client->sendPacketToServer(packet, s_stamp);
	}

	void SandboxScene::processAllPackets(const GameTime& gameTime)
	{
		Packet* packet = nullptr;
		ENetPeer* from;
		while ((packet = s_client->receivePacket(from)) != nullptr)
		{
			// "protocol" messages
			if (packet->getType() <= PacketTypes::Disconnect)
			{
				s_client->processClientSidePackets(packet, from, gameTime.m_total);
				
			}
			else if (packet->getType() <= PacketTypes::LobbyJoinResult)
			{
				s_client->processMatchmakingPackets(packet, from, gameTime.m_total);
			}
			else
				processPacket(packet);

			if (packet->getType() == PacketTypes::Disconnect)
			{
			}
			destroyPacket(packet);
		}
	}
	void SandboxScene::processPacket(Packet* packet)
	{
		switch (packet->getType())
		{
		case PacketTypes::GameSetup:
		{
			GameSetupPacket* setupPacket = (GameSetupPacket*)packet;
			m_playerId = setupPacket->m_clientIndex;
			// TOOD: Make a new packet type that tells client it is connected to lobby.
			sandbox->requestMap(0);
			break;
		}
		case PacketTypes::GameUpdate:
		{
			updateGladiators((GameUpdatePacket*)packet);
			break;
		}
		case PacketTypes::GameCreateGladiators:
		{
			createGladiators((GameCreateGladiatorsPacket*)packet);
			break;
		}
		case PacketTypes::GamePlatform:
		{
			createPlatform((GamePlatformPacket*)packet);
			break;
		}
		case PacketTypes::GameSpawnBullets:
		{
			spawnBullets((GameSpawnBulletsPacket*)packet);
			break;
		}
		case PacketTypes::GameBulletHit:
		{
			spawnBulletHits((GameBulletHitPacket*)packet);
			break;
		}
		case PacketTypes::GameDamagePlayer:
		{
			processDamagePlayer((GameDamagePlayerPacket*)packet);
			break;
		}
		case PacketTypes::GameKillPlayer:
		{
			killPlayer((GameKillPlayerPacket*)packet);
			break;
		}
		case PacketTypes::GameRespawnPlayer:
		{
			respawnPlayer((GameRespawnPlayerPacket*)packet);
			break;
		}
		case PacketTypes::GameUpdateScoreBoard:
		{
			updateScoreBoard((GameUpdateScoreBoardPacket*)packet);
			break;
		}
		case PacketTypes::GameMode:
		{
			setGameMode((GameModePacket*)packet);
			break;
		}
		default:
		{
			printf("Unknown packet type received on sandbox_scene, id: %d\n", packet->getType());
		}
		}
	}

	void SandboxScene::createGladiators(GameCreateGladiatorsPacket* packet)
	{
		m_scoreboard = new Scoreboard;
		for (unsigned i = 0; i < unsigned(packet->m_playerAmount); i++)
		{
			CharacterData characterData = packet->m_characterArray[i];
			Player player;
			CharacterComponent* chara = (CharacterComponent*)m_factory->createGladiator(characterData)->first(TYPEOF(CharacterComponent));
			player.m_gladiator = &chara->m_gladiator;
			player.m_clientIndex = packet->m_characterArray[i].m_ownerId;
			if (player.m_clientIndex == m_playerId)
			{
				player.m_playerController = new PlayerController;
				CameraManager::instance().m_character = chara;
			}
			else
				player.m_playerController = nullptr;
			m_players.push_back(player);
		}
		m_toggleKeyBindDraw = false;
	}
	void SandboxScene::createPlatform(GamePlatformPacket* packet)
	{
		ArenaPlatform platform;
		platform.type = (ArenaPlatformType)packet->m_platform.m_type;
		for (int32_t i = 0; i < packet->m_platform.m_vertexAmount; i++)
		{
			platform.vertices.push_back(packet->m_platform.m_vertexArray[i]);
		}
		m_platformVector.push_back(platform);

		m_physics.createPlatform(platform.vertices, platform.type);
	}
	
	void SandboxScene::updateGladiators(GameUpdatePacket* packet)
	{
		CharacterManager::instance().applyUpdatePacket(packet);
	}

	void SandboxScene::spawnBullets(GameSpawnBulletsPacket* packet)
	{
		// Spawnbullets is used to create and update server side bullets (Projectile).
		// If no match is found it creates a new bullet. This creates both clientside and serverside bullet.
		// Clientside bullets are not registered as Projectiles, but as physics entities.
		for (unsigned i = 0; i < packet->m_bulletAmount; i++)
		{
			bool addNew = true;
			// Check if bullet exists.
			if (debugBullets)
			{
				ProjectileManager& instance = ProjectileManager::instance();
				for (auto it = instance.begin(); it != instance.end(); it++)
				{
					if ((*it)->bullet.getEntityID() == packet->m_bulletSpawnArray[i].m_id)
					{
						*(*it)->bullet.m_position = packet->m_bulletSpawnArray[i].m_position;
						addNew = false;
						break;
					}
				}
			}
			if (addNew)
				createBullet(packet->m_bulletSpawnArray[i]);
		}
	}
	void SandboxScene::spawnBulletHits(GameBulletHitPacket *packet)
	{
		for (unsigned i = 0; i < packet->m_bulletAmount; i++)
		{
			m_factory->createBulletHit(packet->bulletHitArray[i]);
			if (packet->bulletHitArray[i].m_type != 2)
			destroyBullet(packet->bulletHitArray[i].m_id);
		}

	}
	void SandboxScene::processDamagePlayer(GameDamagePlayerPacket* packet)
	{
		CharacterManager::instance().applyDamageToCharacter(packet);
		destroyBullet(packet->m_bulletId);
	}
	void SandboxScene::killPlayer(GameKillPlayerPacket* packet)
	{
		CharacterManager::instance().killCharacter(packet);
	}
	void SandboxScene::respawnPlayer(GameRespawnPlayerPacket* packet)
	{
		CharacterManager::instance().respawnCharacter(packet);
	}
	void SandboxScene::updateScoreBoard(GameUpdateScoreBoardPacket* packet)
	{
		if (m_scoreboard == nullptr)
		{
			m_scoreboard = new Scoreboard;
			m_scoreboard->m_flagHolder = packet->m_scoreBoardData.m_flagHolder;
			for (unsigned i = 0; i < packet->m_playerAmount; i++)
			{ 
				PlayerScore score;
				score.m_score = (packet->m_scoreBoardData.m_playerScoreArray[i].m_score);
				score.m_tickets = (packet->m_scoreBoardData.m_playerScoreArray[i].m_tickets);
				score.m_kills = (packet->m_scoreBoardData.m_playerScoreArray[i].m_kills);
				score.m_playerID = packet->m_scoreBoardData.m_playerScoreArray[i].m_playerID;
				m_scoreboard->m_playerScoreVector.push_back(score);
			}
		}

		m_scoreboard->m_flagHolder = packet->m_scoreBoardData.m_flagHolder;
		bool match = false;
		for (unsigned i = 0; i < packet->m_playerAmount; i++)
		{
			for(unsigned j = 0; j < m_scoreboard->m_playerScoreVector.size(); j++)
			{ 
				// Find the correct id.
				if (m_scoreboard->m_playerScoreVector[j].m_playerID == packet->m_scoreBoardData.m_playerScoreArray[i].m_playerID)
				{ 
					m_scoreboard->m_playerScoreVector[j].m_score = (packet->m_scoreBoardData.m_playerScoreArray[i].m_score);
					m_scoreboard->m_playerScoreVector[j].m_tickets = (packet->m_scoreBoardData.m_playerScoreArray[i].m_tickets);
					m_scoreboard->m_playerScoreVector[j].m_kills = (packet->m_scoreBoardData.m_playerScoreArray[i].m_kills);
					match = true;
				}
			}
			// If there is no correct match for id, add it to array.
			if (!match)
			{
				PlayerScore score;
				score.m_score = (packet->m_scoreBoardData.m_playerScoreArray[i].m_score);
				score.m_tickets = (packet->m_scoreBoardData.m_playerScoreArray[i].m_tickets);
				score.m_kills = (packet->m_scoreBoardData.m_playerScoreArray[i].m_kills);
				score.m_playerID = packet->m_scoreBoardData.m_playerScoreArray[i].m_playerID;
				m_scoreboard->m_playerScoreVector.push_back(score);
			}
		}
	}

	void SandboxScene::setGameMode(GameModePacket* packet)
	{
		int32_t index = packet->m_gameModeIndex;
		m_gameMode = GameModeFactory::createGameModeFromIndex(index, m_scoreboard, &m_physics, &m_players, NULL, NULL);
	}

	void SandboxScene::cleanUp()
	{
		for (auto iterator = entititesBegin(); iterator != entititesEnd(); iterator++)
		{
	
			Entity* entity = (*iterator);
			if (entity->contains(TYPEOF(Id)))
			{ 
				Id* id = (Id*)entity->first(TYPEOF(Id));
				if (id->m_id == EntityIdentification::MousePointer || id->m_id == EntityIdentification::MapBack || id->m_id == EntityIdentification::MapFront||
					id->m_id == EntityIdentification::Platform)
				{
					continue;
				}
			}
			entity->destroy();
		}
		// TODO: Clean up charaters.
		CharacterManager::instance().empty();

	}

	void SandboxScene::updateEntities(const GameTime& gameTime)
	{
		// TODO: Do own systems for these.
		updateServerBullets(gameTime);
		PhysicsManager::instance().update(gameTime);

		Transform* playerTransform = (Transform* const)CharacterManager::instance().getCharacterByPlayerId(m_playerId)->owner()->first(TYPEOF(Transform));
		for (auto iterator = entititesBegin(); iterator != entititesEnd();)
		{
			Entity* entity = *iterator;
			if (entity->contains(TYPEOF(Timer)))
			{
				// Update timer and destroy entities that are out of their lifetime.
				Timer* timer = (Timer*)entity->first(TYPEOF(Timer));
				if (timer->timePassed((float)gameTime.m_delta))
				{
					entity->destroy();
					continue;
				}

				// Fade entities that need to be faded away
				if (entity->contains(TYPEOF(SpriteRenderer)) && entity->contains(TYPEOF(Id)))
				{
					Id* entityId = (Id*)entity->first(TYPEOF(Id));
					// If entity is type of smoke, fade it.
					SpriteRenderer* render = (SpriteRenderer*)entity->first(TYPEOF(SpriteRenderer));
					
					if (entityId->m_id == EntityIdentification::Smoke)
					{
						render->setColor(color::toABGR(255, 255, 255, static_cast<uint8_t>(timer->timePassedReverse255() / 8)));
					}

					if (entityId->m_id == EntityIdentification::GrenadeSmoke)
					{
						render->setColor(color::toABGR(255, 255, 255, static_cast<uint8_t>(timer->timePassedReverse255() / 1.5)));
					}

					if (entityId->m_id == EntityIdentification::Explosion)
					{
						render->setColor(color::toABGR(255, 255, 255, static_cast<uint8_t>(timer->timePassedReverse255() / 1)));
					}
					if (entityId->m_id == EntityIdentification::Magazine)
					{
						// fade out magazine after certain time.
						Timer* timer = (Timer*)entity->first(TYPEOF(Timer));
						if (timer->m_currentTime/timer->m_lifeTime > 0.7f)
						{
							render->setColor(color::toABGR(255, 255, 255, static_cast<uint8_t>(timer->timePassedReverse255() * 2.5)));
						}
					}
				}

				

				// Move entities that need to be moved
				if (entity->contains(TYPEOF(Movement)) && entity->contains(TYPEOF(Transform)) && entity->contains(TYPEOF(SpriteRenderer)))
				{
					Movement* movement = (Movement*)entity->first(TYPEOF(Movement));
					Transform* transform = (Transform*)entity->first(TYPEOF(Transform));
					SpriteRenderer* render = (SpriteRenderer*)entity->first(TYPEOF(SpriteRenderer));


					if (timer->m_between > 0.016f)
					{
						transform->m_position += movement->m_velocity;
						movement->m_velocity = glm::vec2(movement->m_velocity.x *0.9, movement->m_velocity.y*0.9);

						render->setRotation(render->getRotation() + movement->m_rotationSpeed);
						timer->resetBetween();
					}
				}

			}
			if (entity->contains(TYPEOF(BulletTrail)))
			{
				BulletTrail* trail = (BulletTrail*)entity->first(TYPEOF(BulletTrail));
				if (trail->getDone())
				{
					entity->destroy();
					iterator++;
					continue;
				}
				
					if (!m_physics.entityExists(trail->bulletId))
					{
						trail->bulletDestroyed = true;
					}
					else 
					{
						if (m_physicsUpdated)
						{
							glm::vec2 position = m_physics.getEntityPosition(trail->bulletId);
							glm::vec2 velocity = m_physics.getEntityVelocity(trail->bulletId);
							trail->updateHeadData(position, velocity);
						}
						Transform* const transform = new Transform();
						entity->add(transform);

						SpriteRenderer* const renderer = new SpriteRenderer();
						renderer->setTexture(App::instance().resources()->get<TextureResource>(ResourceType::Texture, "effects/trail.png"));
						entity->add(renderer);

						float rotation = m_physics.getEntityVelocityAngle(trail->bulletId);
						trail->addPart( rotation, transform, renderer);
						
					}

				
				trail->update(static_cast<float>(gameTime.m_delta));
			}
			

			if (entity->contains(TYPEOF(Id)))
			{


				Id* id = (Id*)entity->first(TYPEOF(Id));
				if (id->m_id == MapBack)
				{
					Movement* movement = (Movement*)entity->first(TYPEOF(Movement));
					Transform* transform = (Transform*)entity->first(TYPEOF(Transform));
					// Parallex scrolling.
					// use movement component as offset component.
					glm::vec2 mapOffset = glm::vec2(playerTransform->m_position.x *0.25f, playerTransform->m_position.y *0.25f);
					// HAX: velocity is original position, too lazy to create originalposition-component or something like that
					transform->m_position = glm::vec2(movement->m_velocity.x + mapOffset.x, movement->m_velocity.y + mapOffset.y);
				}
				else if (id->m_id == HitBlood)
				{
					//Movement* movement = (Movement*)entity->first(TYPEOF(Movement));
					//					Transform* transform = (Transform*)entity->first(TYPEOF(Transform));
					SpriteRenderer* render = (SpriteRenderer*)entity->first(TYPEOF(SpriteRenderer));
					Timer* timer = (Timer*)entity->first(TYPEOF(Timer));

					// TODO: Remake this.
					float time = timer->timePassed();

					int multipler = 0;

					if (time < 0.10f)
					{
						multipler = 0;
					}
					else if (time < 0.25)
					{
						multipler = 1;
					}
					else if (time < 0.45)
					{
						multipler = 2;
					}
					else if (time < 0.55)
					{
						multipler = 3;
					}
					else if (time < 0.75)
					{
						multipler = 4;
					}

					else if (time < 0.85)
					{
						multipler = 5;
					}

					else if (time < 0.90)
					{
						multipler = 6;
					}

					Rectf& rectangle = render->getSource();
					rectangle.x = 128.0f * multipler;
					rectangle.y = 0.0f;
					rectangle.w = 128.0f * (multipler + 1);
					rectangle.h = 32.0f;

					rectangle = render->getSource();

				}
				else if (id->m_id == Minibomb)
				{
					Timer* timer = (Timer*)entity->first(TYPEOF(Timer));
					if (timer->timePassed() > 0.5f)
					{ 
						PlayerLink* link = (PlayerLink*)entity->first(TYPEOF(PlayerLink));
						CharacterComponent* chara = CharacterManager::instance().getCharacterByPlayerId(link->m_playerId);
						m_factory->createExplosionBlood(*chara->m_gladiator.m_position);
						Animator* animator = (Animator*)chara->owner()->first(TYPEOF(Animator));
						animator->m_animator.hide = true;
						entity->destroy();
					}
				}

				else if (id->m_id == ExplosionBlood)
				{
					//Movement* movement = (Movement*)entity->first(TYPEOF(Movement));
					//					Transform* transform = (Transform*)entity->first(TYPEOF(Transform));
					SpriteRenderer* render = (SpriteRenderer*)entity->first(TYPEOF(SpriteRenderer));
					Timer* timer = (Timer*)entity->first(TYPEOF(Timer));

					// TODO: Remake this.
					float time = timer->timePassed();

					int multiplerX = 0;
					int multiplerY = 0;

					if (time > 0.0f)
					{
						multiplerX = 0;
					}
					if (time < 0.25)
					{
						multiplerX = 1;
					}
					else if (time < 0.45)
					{
						multiplerX = 2;
					}
					else if (time < 0.55)
					{
						multiplerX = 3;
					}
					else if (time < 0.75)
					{
						multiplerX = 0;
						multiplerY = 1;
					}
					else
					{
						multiplerX = 1;
						multiplerY = 1;
					}

					Rectf& rectangle = render->getSource();
					rectangle.x = 256.0f * multiplerX;
					rectangle.y = 256.0f * multiplerY;
					rectangle.w = 256.0f;
					rectangle.h = 256.0f;
				}
				else if (id->m_id == EntityIdentification::BulletModel)
				{
					PhysicsComponent* physics = (PhysicsComponent*)entity->first(TYPEOF(PhysicsComponent));
					uint8_t bulletID = physics->m_physicsId;
					glm::vec2 velocity =m_physics.getEntityVelocity(bulletID);
					// Get combined velocity.
					float vel = sqrt((velocity.x * velocity.x + velocity.y*velocity.y));
					// Destroy bullet if it is slow enought.
					if (vel < 1200)
					{ 
						SpriteRenderer* render = (SpriteRenderer*)entity->first(TYPEOF(SpriteRenderer));
						float alpha = vel - 945;
						render->setColor(color::toABGR(255, 255, 255, static_cast<uint8_t>(alpha)));
					}
					if (vel < 800)
						destroyBullet(bulletID);

				}


			}
			iterator++;
		}
	}
	void SandboxScene::updateServerBullets(const GameTime& gameTime)
	{
		ProjectileManager& instance = ProjectileManager::instance();
		instance.update(gameTime);
	}

	void SandboxScene::updatePhysics(float32 timeStep)
	{
		//m_physics.update(gameTime.m_delta);
		m_physics.update(timeStep);
	
	}

	void SandboxScene::destroyBullet(uint8_t bulletId)
	{
		for (auto it = entititesBegin(); it != entititesEnd(); ++it)
		{
			Entity* entity = *it;
		
			if (entity->contains(TYPEOF(PhysicsComponent)))
			{
				PhysicsComponent *component = (PhysicsComponent*)entity->first(TYPEOF(PhysicsComponent));
				if (component->m_physicsId == bulletId)
				{
					m_factory->freeEntityId(bulletId);
					m_physics.removeEntity(component->m_physicsId);
					entity->destroy();	
				}
			}
			if (entity->contains(TYPEOF(Projectile)))
			{
				Projectile *projectile = (Projectile*)entity->first(TYPEOF(Projectile));
				if(projectile->bullet.getEntityID() == bulletId)
				{ 
					m_factory->freeEntityId(bulletId);
					entity->destroy();
				}
			}
		
		}
	}

	void SandboxScene::createBullet(BulletData &data)
	{
		CharacterManager::instance().characterShoot(data);
	}
	
	void SandboxScene::checkBounds(glm::vec2& cameraPosition)
	{
		if (cameraPosition.x < m_screenSize.x / 2)
		{
			cameraPosition.x = m_screenSize.x / 2;
		}
		else if (cameraPosition.x > m_screenSize.x * 3.5f)
		{
			cameraPosition.x = m_screenSize.x * 3.5f;
		}
		if (cameraPosition.y < m_screenSize.y / 2)
		{
			cameraPosition.y = m_screenSize.y / 2;
		}
		else if (cameraPosition.y > m_screenSize.y * 1.5f)
		{
			cameraPosition.y = m_screenSize.y* 1.5f;
		}
	}



	void SandboxScene::setDrawText(const GameTime& gameTime)
	{
		bgfx::dbgTextClear();
		unsigned row = 0;
		gameTime;
		//const MouseState& mouse = Mouse::getState();
		//glm::vec2 mouseLoc(mouse.m_mx, mouse.m_my);
		//bgfx::dbgTextPrintf(0, row, 0x9f, "Delta time %.10f", gameTime.m_delta);
		//row++;
		//bgfx::dbgTextPrintf(0, row, 0x8f, "Left btn = %s, Middle btn = %s, Right btn = %s",
		//row++;
		//	mouse.m_buttons[MouseButton::Left] ? "down" : "up",
		//	mouse.m_buttons[MouseButton::Middle] ? "down" : "up",
		//	mouse.m_buttons[MouseButton::Right] ? "down" : "up");
		//bgfx::dbgTextPrintf(0, row, 0x6f, "Mouse (screen) x = %d, y = %d, wheel = %d", mouse.m_mx, mouse.m_my, mouse.m_mz);
		//row++;
		//bgfx::dbgTextPrintf(0, row, 0x9f, "Mouse pos (world) x= %.2f, y=%.2f", mouseLoc.x, mouseLoc.y);
		//row++;
		//bgfx::dbgTextPrintf(0, row, 0x9f, "Angle (%.3f rad) (%.2f deg)", m_controller.aimAngle, glm::degrees(m_controller.aimAngle));
		//row++;
		if (m_toggleKeyBindDraw == true)
		{

			bgfx::dbgTextPrintf(0, row++, 0x9f, "Q: Connect to game");
			bgfx::dbgTextPrintf(0, row++, 0x9f, "9: Disconnect from game");
			bgfx::dbgTextPrintf(0, row++, 0x9f, "Left mousebutton: Shoot");
			bgfx::dbgTextPrintf(0, row++, 0x9f, "R: Reload");
			bgfx::dbgTextPrintf(0, row++, 0x9f, "Right mousebutton: Throw grenade");
			bgfx::dbgTextPrintf(0, row++, 0x9f, "A: Move left");
			bgfx::dbgTextPrintf(0, row++, 0x9f, "D: Move right");
			bgfx::dbgTextPrintf(0, row++, 0x9f, "W: Climb ladder up");
			bgfx::dbgTextPrintf(0, row++, 0x9f, "S: Climb ladder down / drop down platform");
			bgfx::dbgTextPrintf(0, row++, 0x9f, "Space: Jump");
			bgfx::dbgTextPrintf(0, row++, 0x9f, "F1: show/hide this");
			bgfx::dbgTextPrintf(0, row++, 0x9f, "Tab: show/hide scoreboard");
			bgfx::dbgTextPrintf(0, row++, 0x9f, "+: Add player slot on lobby");
			bgfx::dbgTextPrintf(0, row++, 0x9f, "-: Remove player slot on lobby");
			if (s_client->isConnected())
				bgfx::dbgTextPrintf(0, row++, 0x56f, "Connected", s_client->isConnected());
			else
				bgfx::dbgTextPrintf(0, row++, 0x9f, "Disconnected", s_client->isConnected());

		}

		row++;
		if (m_scoreboard != nullptr && m_toggleScoreboardDraw)
		{

			for (const auto& elem : m_scoreboard->m_playerScoreVector)
			{
				bgfx::dbgTextPrintf(0, row, 0x8f, "Player: %d: \t Score: %d \t Kills: %d \t Tickets %d",
					elem.m_playerID, elem.m_score, elem.m_kills, elem.m_tickets);
				row++;
			}
			row++;
			row++;
			// TODO: Draw this if there is need for it.
			//for (auto i = m_clientIdToGladiatorData.begin(); i != m_clientIdToGladiatorData.end(); i++) {
			//	Gladiator* gladiator = i->second->m_gladiator;
			//	bgfx::dbgTextPrintf(0, row, 0x8f, "Player: %d: \t Team: %d",
			//		i->first, gladiator->m_team);
			//	row++;
			//}
		}
		if (m_gameMode == nullptr)
			return;
		//show game end
		if (m_gameMode->isEnd())
		{
			int col = 100;
			bgfx::dbgTextPrintf(col, 20, 0x9f, "Round ends");
			

			std::vector<std::string> internal;
			std::stringstream ss(m_gameMode->getEndMessage()); // Turn the string into a stream.
			std::string tok;

			while (getline(ss, tok,'=')) {
				internal.push_back(tok);
				bgfx::dbgTextPrintf(col, 20, 0x9f, tok.c_str());
			}
		}
		//else
		//	bgfx::dbgTextPrintf(0, row++ + 10, 0x9f, "GAME END: FALSE");
	}
	void SandboxScene::createBackground()
	{
		ResourceManager* resources = App::instance().resources();
		(void)resources;
		EntityBuilder builder;

		// Create back
		if (m_backgroundSetting > 1)
		{

			for (int y = 0; y <= 3; y++)
			{
				for (int x = 0; x <= 5; x++)
				{
					// TODO: add offset component.
					builder.begin();
					Movement* move = builder.addMovement();
					move->m_velocity = glm::vec2(0, 0);
					move->m_rotationSpeed = 0;
					builder.addIdentifier(MapBack);
					std::string name = "map/B";
					name += std::to_string(x);
					name += std::to_string(y);
					name += ".png";

					SpriteRenderer* renderer = builder.addSpriteRenderer();
					TextureResource* textureResource = resources->get<TextureResource>(ResourceType::Texture, name);
					Transform* transform = builder.addTransformComponent();

					transform->m_position = glm::vec2((x - 1) * 1920, (y - 1) * 1080);
					renderer->setTexture(textureResource);
					move->m_velocity = transform->m_position;
					renderer->setLayer(0);
					renderer->anchor();
					registerEntity(builder.getResults());
				}
			}
		}

		// Create front
		if (m_backgroundSetting == 1 || m_backgroundSetting == 3)
		{

			for (unsigned y = 0; y <= 1; y++)
			{
				for (unsigned x = 0; x <= 3; x++)
				{
					builder.begin();
					builder.addIdentifier(MapFront);
					SpriteRenderer* renderer = builder.addSpriteRenderer();

					std::string name = "map/";
					name += std::to_string(x);
					name += std::to_string(y);
					name += ".png";

					TextureResource* textureResource = resources->get<TextureResource>(ResourceType::Texture, name);

					Transform* transform = builder.addTransformComponent();

					transform->m_position = glm::vec2(x * 1920, y * 1080);

					renderer->setTexture(textureResource);
					renderer->setLayer(1);
					renderer->anchor();
					registerEntity(builder.getResults());
				}
			}
		}

	}
	
}
