#pragma once

#include <Trackable.h>
#include <Vector2D.h>




class System;
class Unit;
class GraphicsBuffer;
class UnitManager;
class MemoryManager;
class GraphicsBufferManager;
class NetworkManager;
typedef std::string GBKey;

class Game : public Trackable
{
public:

	static Game* getInstance();
	static void initInstance();
	static void deleteInstance();

	bool init(unsigned int width, unsigned int height, double targetTimePerFrame = 16.7);
	void cleanup();

	void doLoop();

	System* getSystem() const { return mpSystem; };

	double getTargetTimePerFrame() const { return mTargetTimePerFrame; };
	GraphicsBufferManager* getGraphicsBufferManager() const { return mpGraphicsBufferManager; };
	UnitManager* getUnitManager() const { return mpUnitManager; };
	MemoryManager* getMemoryManager() const { return mpMemoryManager; };


private:
	static Game* mspInstance;

	System* mpSystem=NULL;
	UnitManager* mpUnitManager = NULL;
	GraphicsBufferManager* mpGraphicsBufferManager = NULL;
	MemoryManager* mpMemoryManager = NULL;

	double mTargetTimePerFrame=0;
	bool mIsInitted = false;
	bool mShouldContinue = true;

	NetworkManager* mpNetworkManager = NULL;



	//private constructor/destructor - don't want someone trying to delete the instance directly
	Game();
	~Game();

	//might as well invalidate copy constructor and assignment operator
	Game(Game& game);
	void operator=(const Game& game);

	void getInput();
	void update(double dt);
	void render();

	void loadBuffers();
	void createUnit(const Vector2D& pos);
};


const GBKey WOODS("woods");
const GBKey SMURFS("smurfs");
const GBKey DEAN("dean");
