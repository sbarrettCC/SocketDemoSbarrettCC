#pragma once

/*
	File information:
	This file contains the definition for the Wall class, derived from GameObject
*/

#include "GameObject.h"

class Wall : public GameObject
{
	//-------------------------Private data-------------------------
	float mSizeX;
	float mSizeY;


	//-------------------------Public data-------------------------
public:

	//Constructor(s)
	Wall(const int gameObjectID, const int networkID);
	Wall(const int gameObjectID, const int networkID, pair<float, float> position, float sizeX, float sizeY);

	//Destructor
	~Wall();

	//Accessor(s)
	float getWallSizeX() { return mSizeX; };
	float getWallSizeY() { return mSizeY; };

	//Mutator(s)


	//Functions
	void update();
	void draw();
};