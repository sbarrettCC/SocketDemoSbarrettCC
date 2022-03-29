#pragma once
#include <string>

class GraphicsLibrary;

class GameObject
{
	private:
	protected:
		GameObject(std::string graphicsID, int posX, int posY, int width, int height);
		GameObject(std::string graphicsID, int width, int height);
		Quaternion mGraphicsData; //X,Y Position Z,W Sizing
		std::string* mpGraphicsID;
	public:
		enum { mClassId = 'GOBJ' };
		virtual uint32_t GetClassId() const { return mClassId; }
		virtual void Update() = 0;
		virtual void Write(OutputMemoryBitStream &stream) = 0;
		virtual void Read(InputMemoryBitStream &stream) = 0;
		virtual void Render(GraphicsLibrary* gl);
		void SetPostion(int posX, int posY);
		~GameObject();
};
struct Location
{
	float x;
	float y;
};
#define CLASS_IDENTIFICATION( inCode, inClass ) \
enum{ mClassId = inCode }; \
virtual uint32_t GetClassId() const { return mClassId; } \
static GameObject* CreateInstance() { return new inClass(); }