#pragma once

#include <queue>
#include <vector>
#include <enet\enet.h>
#include <bx/thread.h>

// Help: http://enet.bespin.org/Tutorial.html#Connecting

struct Message
{
    unsigned char* data;
    size_t size;
    bool reliable;
};

class Network
{
	public:
		Network();
		~Network();
		
		void connectServer(char* address, unsigned port);
		void checkEvent();
        void checkSendQueue();
		void sendPacket(unsigned char* data, size_t size, bool reliable = true);
		void sendMessages();

		void disconnect();
		void setMessageQueue(std::queue<unsigned char*> *messageQueue);
        void setSendQueue(std::queue<Message> *sendQueue);
        void setMutex(bx::Mutex *rMutex, bx::Mutex *sMutex);
	private:
		void initializeENet();
		ENetHost* createENetClient();
		ENetHost* m_client;
		ENetPeer* m_peer;
		std::queue<unsigned char*>* m_messageQueue;
        std::queue<Message>* m_sendQueue;
        bx::Mutex* m_sMutex, *m_rMutex;
};
