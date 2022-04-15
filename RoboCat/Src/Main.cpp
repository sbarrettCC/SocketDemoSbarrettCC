
#include "RoboCatPCH.h"
#include "allegro5.h"
#include "allegro_primitives.h"
#include "allegro_color.h"
#include "RectangleObject.h"
#include "RainParticle.h"
#include "CircleClass.h"
#include "math.h"

#include <thread>
#include <iostream>
#include <string>
#include <sstream>
#include <list>

using namespace std;
int windowWidth = 800;
int windowHeight = 600;
int FRAMERATE = 16;

#if _WIN32


class Packet
{
public:
	Packet();
	Packet(double stamp, char* buffer, int byteCount);
	~Packet();
	double timeStamp;
	char* mBufferPtr;
	int mByteCount;
private:

};
Packet::Packet(double stamp, char* buffer, int byteCount) {
	timeStamp = stamp;
	mBufferPtr = buffer;
	mByteCount = byteCount;
};
//Packet::Packet(double stamp, InputMemoryBitStream bitstream) {
//	timeStamp = stamp;
//	bitStream = &bitstream;
//};
Packet::Packet()
{
	timeStamp = 0;
	mBufferPtr = new char[4096];
	mByteCount = 4096;
	//bitStream = new InputMemoryBitStream(buffer, 4096);
}

Packet::~Packet()
{
}


void ThrowSocketError(std::string errorMsg)
{
	SocketUtil::ReportError(errorMsg.c_str());
	ExitProcess(1); // kill
}

ALLEGRO_TIMER* GiveMeATimer(float incr)
{
	ALLEGRO_TIMER* timer = al_create_timer(incr);
	if (timer == nullptr)
	{
		ERROR;
	}
	al_start_timer(timer);
	return timer;
}


TCPSocketPtr StartServer()
{
	TCPSocketPtr listeningSocket = SocketUtil::CreateTCPSocket(SocketAddressFamily::INET);
	if (listeningSocket == nullptr)
		ThrowSocketError("Could not create socket: NullPtr");

	SocketAddressPtr addressPtr = SocketAddressFactory::CreateIPv4FromString("0.0.0.0:8080"); // a hard-coded port to bind to
	if (addressPtr == nullptr)
		ThrowSocketError("Could not create server address");

	bool bindError = listeningSocket->Bind(*addressPtr) != NO_ERROR; // bind the socket and check if there is an error
	if (bindError)
		ThrowSocketError("Could not bind to address");

	bool listenError = listeningSocket->Listen() != NO_ERROR;
	if (listenError)
		ThrowSocketError("Listen Error");

	LOG("%s", "Waiting to accept connections...");

	SocketAddress incomingAddress;
	TCPSocketPtr incomingSocket = listeningSocket->Accept(incomingAddress);
	while (incomingSocket == nullptr) // if it didnt work the first time, try again
	{
		incomingSocket = listeningSocket->Accept(incomingAddress);
	}
	LOG("Request accepted from: %s", incomingAddress.ToString().c_str());

	return incomingSocket;
}

TCPSocketPtr StartClientConnection()
{
	TCPSocketPtr clientSocket = SocketUtil::CreateTCPSocket(SocketAddressFamily::INET);
	if (clientSocket == nullptr)
		ThrowSocketError("Could not create client socket");

	std::string address = StringUtils::Sprintf("127.0.0.1:8081");
	SocketAddressPtr clientAddress = SocketAddressFactory::CreateIPv4FromString(address.c_str());
	if (clientAddress == nullptr)
		ThrowSocketError("Creating client address");

	if (clientSocket->Bind(*clientAddress) != NO_ERROR)
		ThrowSocketError("Binding Client Socket");

	SocketAddressPtr serverAddress = SocketAddressFactory::CreateIPv4FromString("127.0.0.1:8080");
	if (serverAddress == nullptr)
		ThrowSocketError("Creating server address");

	if (clientSocket->Connect(*serverAddress) != NO_ERROR)
		ThrowSocketError("Connecting to server");

	return clientSocket;
}

void BradsTotallyOriginalServer()
{
	TCPSocketPtr incomingSocket = StartServer();
	incomingSocket->SetNonBlockingMode(true);

	//Timer
	ALLEGRO_TIMER* timer = GiveMeATimer(0.001);
	double currentTime = al_get_timer_count(timer);
	float lastTime = 0;
	float deltaTime = 0;

	// Game Loop Variables
	bool exit = false;
	const int numObjects = 1;

	//Greate GameObjects
	CircleClass greenCircle[numObjects];
	for (int j = 0; j < numObjects; j++)
	{
		greenCircle[j] = CircleClass("serverPlayer", windowWidth, windowHeight, 100, 100, 20);
	}

	const int numDroplets = 10;
	RainParticle rain[numDroplets];
	for (int k = 0; k < numDroplets; k++)
	{
		rain[k] = RainParticle(windowWidth, windowHeight, rand() % windowWidth + 1, rand() % windowHeight + 1, rand() % 5 + 5, 0, 0, rand() % 254 + 1, rand() % 254 + 1);
	}

	RectangleObject inObjects[numObjects];
	list < RectangleObject* > clientProjectiles;
	list < CircleClass* > serverProjectiles;


	ALLEGRO_DISPLAY* display;
	display = al_create_display(windowWidth, windowHeight);

	ALLEGRO_KEYBOARD_STATE keyState;
	ALLEGRO_EVENT_QUEUE* eventQueue = al_create_event_queue();
	al_register_event_source(eventQueue, al_get_keyboard_event_source());

	int xAxis = 0;
	int yAxis = 0;

	bool xMove = true;
	bool yMove = true;

	int projShot = 0;

	while (!exit) //GameLoop
	{
		//Timer
		currentTime = al_get_timer_count(timer);
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		//send Data
		OutputMemoryBitStream oStream = OutputMemoryBitStream();

		for (int i = 0; i < numObjects; i++)
		{
			greenCircle[i].Write(oStream);
		}

		for (int j = 0; j < numDroplets; j++)
		{
			rain[j].Write(oStream);
		}

		int projectileCount = serverProjectiles.size();
		oStream.Write(projectileCount);
		for each (CircleClass * projectile in serverProjectiles)
		{
			projectile->Write(oStream);
		}
		//write ack
		incomingSocket->Send(oStream.GetBufferPtr(), oStream.GetByteLength());

		//recieve Data
		char buffer[4096];
		int32_t bytesReceived = int32_t(); // get username
		InputMemoryBitStream iStream = InputMemoryBitStream(buffer, 4096);

		bytesReceived = incomingSocket->Receive(buffer, 4096);
		if (bytesReceived != -10035)
		{
			for (int i = 0; i < numObjects; i++)
			{
				inObjects[i].Read(iStream);
			}
			int clientProjectilesCount;
			iStream.Read(clientProjectilesCount);
			clientProjectiles.clear();
			for (int i = 0; i < clientProjectilesCount; i++)
			{
				RectangleObject* temp = new RectangleObject();
				temp->Read(iStream);
				clientProjectiles.emplace_back(temp);
			}
		}

		ALLEGRO_EVENT events;

		al_get_next_event(eventQueue, &events);
		if (events.type == ALLEGRO_EVENT_KEY_UP)
		{
			switch (events.keyboard.keycode)
			{
			case ALLEGRO_KEY_ESCAPE:
				exit = true;
			case ALLEGRO_KEY_UP:
				yMove = false;
				yAxis = 0;
			case ALLEGRO_KEY_DOWN:
				yMove = false;
				yAxis = 0;
			case ALLEGRO_KEY_RIGHT:
				xAxis = 0;
				xMove = false;
			case ALLEGRO_KEY_LEFT:
				xAxis = 0;
				xMove = false;
			}		
		}
		if (events.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			al_get_keyboard_state(&keyState);
			if (al_key_down(&keyState, ALLEGRO_KEY_UP))
			{
				yMove = true;
				yAxis = -1;
			}
			if (al_key_down(&keyState, ALLEGRO_KEY_DOWN))
			{
				yMove = true;
				yAxis = 1;
			}
			if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT))
			{
				xAxis = 1;
				xMove = true;
			}
			if (al_key_down(&keyState, ALLEGRO_KEY_LEFT))
			{
				xAxis = -1;
				xMove = true;
			}
			if (al_key_down(&keyState, ALLEGRO_KEY_SPACE))
			{
				++projShot;
				CircleClass* proj = new CircleClass(to_string(projShot), windowWidth, windowHeight, greenCircle[0].position[0], greenCircle[0].position[1], 10);
				serverProjectiles.emplace_back(proj);
			}
		}

		for (int i = 0; i < numObjects; i++)
		{
			if (xMove || yMove)
			{
				greenCircle[i].UpdatePos(xAxis, yAxis);
			}
			inObjects[i].Draw();
			greenCircle[i].Draw();
		}
		for each (RectangleObject * projectile in clientProjectiles)
		{
			projectile->Draw();
		}
		for each (CircleClass * projectile in serverProjectiles)
		{
			projectile->UpdatePos(0, -1);
			projectile->Draw();
			for each (RectangleObject* rects in clientProjectiles)
			{
				int xDist = rects->xPos - projectile->position[0];
				int yDist = rects->yPos - projectile->position[1];
				float dist = sqrt(exp2(xDist) + exp2(yDist));
				if (dist < 10)
				{
					/*clientProjectiles.remove(rects);
					delete(rects);
					serverProjectiles.remove(projectile);
					delete(projectile);*/
				}
			}
		}	
		for (int j = 0; j < numDroplets; j++)
		{
			rain[j].UpdatePos(1, 3);
			rain[j].Draw();
		}
		al_flip_display();
		al_clear_to_color(al_map_rgb(0,0,0));

		//wait for the frame to end
		int currentPeriod = al_get_timer_count(timer) - currentTime;
		while (currentPeriod < FRAMERATE)
		{
			currentPeriod = al_get_timer_count(timer) - currentTime;
		}
		if (currentTime > 600000)
			al_set_timer_count(timer, 0);
	}
	al_destroy_display(display);	
}


void BradsLessOriginalClient()
{

	TCPSocketPtr clientSocket = StartClientConnection();
	clientSocket->SetNonBlockingMode(true);

	//timer	
	ALLEGRO_TIMER* timer = GiveMeATimer(0.001);
	double cTime = al_get_timer_count(timer);
	float lastTime = 0;
	float deltaTime = 0;

	bool exit = false;
	const int numObjects = 1;

	//create Game Objects
	RectangleObject outRectangles[numObjects];

	for (int j = 0; j < numObjects; j++)
	{
		outRectangles[j] = RectangleObject("clientPlayer", windowWidth, windowHeight, 20, 20, rand() % windowWidth + 1, rand() % windowHeight + 1);
	}

	// Read In Objects
	CircleClass inObjects[numObjects];

	list < RectangleObject* > clientProjectiles;
	list < CircleClass* > serverProjectiles;

	const int numDroplets = 10;
	RainParticle rain[numDroplets];
	
	// Main Game Loop
	ALLEGRO_DISPLAY* display;
	display = al_create_display(windowWidth, windowHeight);

	ALLEGRO_KEYBOARD_STATE keyState;
	ALLEGRO_EVENT_QUEUE* eventQueue = al_create_event_queue();
	al_register_event_source(eventQueue, al_get_keyboard_event_source());

	int xAxis = 0;
	int yAxis = 0;

	bool xMove = true;
	bool yMove = true;

	int projShot = 0;

	list<Packet*> packets;

	while (!exit) //GameLoop
	{
		//Timer
		cTime = al_get_timer_count(timer);
		deltaTime = cTime - lastTime;
		lastTime = cTime;

		//send Data
		OutputMemoryBitStream oStream = OutputMemoryBitStream();

		for (int i = 0; i < numObjects; i++)
		{
			outRectangles[i].Write(oStream);
		}
		int projectileCount = clientProjectiles.size();
		oStream.Write(projectileCount);
		for each (RectangleObject* projectile in clientProjectiles)
		{
			projectile->Write(oStream);
		}
		//Write ack onto packet
		clientSocket->Send(oStream.GetBufferPtr(), oStream.GetByteLength());

		// Recieve data
		char buffer[4096];
		//PLEASGOD//InputMemoryBitStream iStream = InputMemoryBitStream(buffer, 4096);
		int32_t bytesReceived = int32_t();
		bytesReceived = clientSocket->Receive(buffer, 4096);

		if (bytesReceived != -10035)
		{
			//LOG("%s", "Bytes were recieved"); // this worked

			double currentTime = al_get_timer_count(timer);
			int randomSin = rand() % 2;
			if (randomSin == 0)
			{
				//LOG("%s", "positive jitter")
				currentTime += rand() % 400 + 1; // jitter
			}
			else
			{
				//LOG("%s", "negative jitter")

				currentTime -= rand() % 400 + 1; // jitter
			}
			currentTime += 5000; //latency
			Packet* pack = new Packet(currentTime, buffer, 4096);
			packets.push_back(pack);
			packets.sort([](const Packet* a, const Packet* b) { return a->timeStamp < b->timeStamp; });	
		}	

		if ((int)packets.size() > 0)
		{

			bool processOrNot = false;

			processOrNot = packets.front()->timeStamp < al_get_timer_count(timer);
			cout << "timestamp " << packets.front()->timeStamp << endl;
			cout << "current time " << al_get_timer_count(timer) << endl;
			//cout << "process: " << processOrNot << endl;
			if (processOrNot) // maybe while
			{
				LOG("%s", "process a packet");

				InputMemoryBitStream packStream(packets.front()->mBufferPtr, 4096);
				for (int i = 0; i < numObjects; i++)
				{
					inObjects[i].Read(packStream);
				}
				for (int j = 0; j < numDroplets; j++)
				{
					rain[j].Read(packStream);
				}
				int serverProjectilesCount;
				packStream.Read(serverProjectilesCount);
				serverProjectiles.clear();
				for (int i = 0; i < serverProjectilesCount; i++)
				{
					CircleClass* temp = new CircleClass();
					temp->Read(packStream);
					serverProjectiles.emplace_back(temp);
				}
				packets.pop_front();
				//readack
			}

		}
		ALLEGRO_EVENT events;

		al_get_next_event(eventQueue, &events);

		if (events.type == ALLEGRO_EVENT_KEY_UP)
		{
			switch (events.keyboard.keycode)
			{
			case ALLEGRO_KEY_ESCAPE:
				exit = true;
			case ALLEGRO_KEY_UP:
				yMove = false;
				yAxis = 0;
			case ALLEGRO_KEY_DOWN:
				yMove = false;
				yAxis = 0;
			case ALLEGRO_KEY_RIGHT:
				xAxis = 0;
				xMove = false;
			case ALLEGRO_KEY_LEFT:
				xAxis = 0;
				xMove = false;
			}
		}
		if (events.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			al_get_keyboard_state(&keyState);
			if (al_key_down(&keyState, ALLEGRO_KEY_UP))
			{
				yMove = true;
				yAxis = -1;
			}
			if (al_key_down(&keyState, ALLEGRO_KEY_DOWN))
			{
				yMove = true;
				yAxis = 1;
			}
			if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT))
			{
				xAxis = 1;
				xMove = true;
			}
			if (al_key_down(&keyState, ALLEGRO_KEY_LEFT))
			{
				xAxis = -1;
				xMove = true;
			}
			if (al_key_down(&keyState, ALLEGRO_KEY_SPACE))
			{
				++projShot;
				RectangleObject* proj = new RectangleObject(to_string(projShot), windowWidth, windowHeight, 10, 10, outRectangles[0].xPos, outRectangles[0].yPos);
				clientProjectiles.emplace_back(proj);
			}
		}

		for (int i = 0; i < numObjects; i++)
		{
			if (xMove || yMove)
			{
				outRectangles[i].UpdatePos(xAxis, yAxis);
			}
			outRectangles[i].Draw();
			inObjects[i].Draw();
		}
		for each (CircleClass * projectile in serverProjectiles)
		{
			projectile->Draw();
		}
		for each (RectangleObject* projectile in clientProjectiles)
		{
			projectile->UpdatePos(0, 1);
			projectile->Draw();
		}

		for (int j = 0; j < numDroplets; j++)
		{
			rain[j].Draw();
		}
		al_flip_display();
		al_clear_to_color(al_map_rgb(0, 0, 0));

		//wait for the frame to end
		int currentPeriod = al_get_timer_count(timer) - cTime;
		while (currentPeriod < FRAMERATE)
		{
			currentPeriod = al_get_timer_count(timer) - cTime;
		}
		if (cTime > 600000)
			al_set_timer_count(timer, 0);
	}
	al_destroy_display(display);
}

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
	if (!al_init()) return -1;

	al_init_primitives_addon();

	al_install_keyboard();

	srand(time(NULL));

	bool isServer = StringUtils::GetCommandLineArg(1) == "server"; // check if the command on the executable is 'server'
	if (isServer) 
	{
		BradsTotallyOriginalServer();
	}
	else
	{
		BradsLessOriginalClient();
	}

	SocketUtil::CleanUp(); // socket cleanup
	return 0;
}

