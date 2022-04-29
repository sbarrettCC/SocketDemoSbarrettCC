
#include "RoboCatPCH.h"
#include <thread>
#include "RoboCat/Game.h"
#include"./RoboCat/EventSystem.h"
#include "./common/DeanLib/include/MemoryTracker.h"
#include "./RoboCat/NetworkManager.h"

#if _WIN32

void UDPServer(UDPSocketPtr server)
{
	std::thread sendInput([&] {

	});

	std::thread update([&] {

	});

	sendInput.join();
	update.join();
}

void UDPClient(UDPSocketPtr client)
{
	std::thread sendInput([&] {

	});

	std::thread update([&] {

	});

	sendInput.join();
	update.join();
}

const int DISPLAY_WIDTH = 800;
const int DISPLAY_HEIGHT = 600;

int main(int argc, const char** argv)
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
#else
const char** __argv;
int __argc;
int main(int argc, const char** argv)
{
	__argc = argc;
	__argv = argv;
#endif

	SocketUtil::StaticInit();

	UDPSocketPtr cliSock = SocketUtil::CreateUDPSocket(SocketAddressFamily::INET);
	UDPSocketPtr srvSock = SocketUtil::CreateUDPSocket(SocketAddressFamily::INET);

	SocketAddressPtr srvAddr = SocketAddressFactory::CreateIPv4FromString("127.0.0.1:9001");
	{
		SocketAddressPtr cliAddr = SocketAddressFactory::CreateIPv4FromString("127.0.0.1:9000");
		if (cliAddr == nullptr)
		{
			SocketUtil::ReportError("Create client socket");
			ExitProcess(1);
		}
		cliSock->Bind(*cliAddr);
	}
	srvSock->Bind(*srvAddr);

	std::string msg("Hello server!");
	int bytesSent = cliSock->SendTo(msg.c_str(), msg.length(), *srvAddr);
	if (bytesSent <= 0)
	{
		SocketUtil::ReportError("Client Sendto");
	}
	std::cout << "Sent: " << bytesSent << std::endl;

	std::thread srvThread([&srvSock]() {
		char buffer[4096];
		SocketAddress fromAddr;
		int bytesReceived = srvSock->ReceiveFrom(buffer, 4096, fromAddr);
		if (bytesReceived <= 0)
		{
			SocketUtil::ReportError("Server ReceiveFrom");
			return;
		}
		std::string msg(buffer, bytesReceived);
		std::cout << "Received message from " << fromAddr.ToString() << ": " << msg << std::endl;
	});

	srvThread.join();

	SocketUtil::CleanUp();

	Game::initInstance();
	Game* instance = Game::getInstance();

	instance->init();

	instance->doLoop();

	instance->cleanup();
	Game::deleteInstance();
	EventSystem::cleanupInstance();

	cout << endl;

	MemoryTracker::getInstance()->reportAllocations(cout);


	//Assignment 2 code here
	NetworkManager* mpNetManager = new NetworkManager();

	std::string serverPort = "8080";
	std::string clientIP = "127.0.0.1";
	std::string clientPort = "8081";

	bool serverInited = mpNetManager->initServer(serverPort);
	bool clientInited = mpNetManager->connect(clientIP, clientPort);

	if (serverInited && clientInited)
	{

	}

	system("pause");

	return 0;
}
