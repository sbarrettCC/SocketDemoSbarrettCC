#pragma once
#include "..\..\RoboCat\Inc\RoboCatShared.h"
#include <string>


enum Packet_Header
{
	INVALID_PACKET_TYPE = -1,
	PLAYER_MOVE,
	FIRE_PROJECTILE,
	OBJECT_MOVE,
	OBJECT_DELETE,
	CAMERA_MOVE,
	MAX_PACKET_TYPES
};

class TCPNetworkManager
{

public:
	TCPNetworkManager();
	~TCPNetworkManager();

	void init(string address, int port);
	void cleanup();

	static TCPNetworkManager* getInstance();
	static void cleanupInstance();

	void connectTo(std::string address, int port);
	void listenAndAccept();

	
	void sendPacket(Packet_Header, char* data, int length);
	
	void receivePackets(void (*handleFunction)(Packet_Header header, char* data, int length));

private:
	void establishTCPSocket(std::string address, int port);

	void sendMessage(char* data, int length);
	void sendMessage(string message);
	string receiveMessage();

	static TCPNetworkManager* mspInstance;

	TCPSocketPtr mSocket;
	TCPSocketPtr mConnection;

};