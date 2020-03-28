#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include <graphics/mesh_instance.h>
#include <box2d/Box2D.h>
#include <input/input_manager.h>

enum OBJECT_TYPE
{
	PLAYER,
	ENEMY,
	BULLET
};

class GameObject : public gef::MeshInstance
{
public:
	void UpdateFromSimulation(const b2Body* body);
	void MyCollisionResponse();
	void setInputMan(gef::InputManager inInputMan);

	inline void set_type(OBJECT_TYPE type) { type_ = type; }
	inline OBJECT_TYPE type() { return type_; }


private:
	OBJECT_TYPE type_;
	gef::InputManager* inputMan;
};

class Player : public GameObject
{
public:
	Player();
	void DecrementHealth();
	void playerUpdate(float dt);
	b2Vec2 moveVelocity;
	b2Vec2 jumpVelocity;
};

#endif // _GAME_OBJECT_H