#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include <graphics/mesh_instance.h>
#include <box2d/Box2D.h>
#include <input/input_manager.h>
#include "primitive_builder.h"
#include <vector>

enum OBJECT_TYPE
{
	PLAYER,
	ENEMY,
	BULLET,
	FLOOR
};

class GameObject : public gef::MeshInstance
{
public:
	void UpdateFromSimulation(const b2Body* body);
	void MyCollisionResponse();


	/*Getters 'n' Setters*/
	void setInputMan(gef::InputManager* inInputMan);
	void setPrimitiveBuilder(PrimitiveBuilder* inPrimitiveBuilder);
	void setWorld(b2World* inWorld);

	inline void set_type(OBJECT_TYPE type) { type_ = type; }
	inline OBJECT_TYPE type() { return type_; }

	gef::InputManager* inputManP;
	PrimitiveBuilder* primitiveBuilderP;
	b2World* worldP;

private:
	OBJECT_TYPE type_;

};

class Bullet : public GameObject
{
public:
	Bullet();
	void bulletUpdate(float dt);
	void die();// despawns the bullet

	gef::Vector4 moveVelocity;// the bullet can miss by going off to the side hence it's a vector 4
	float damage;// the damage a bullet does on impact to a gameobject

};

class Player : public GameObject
{
public:
	Player();
	void playerInit();
	void DecrementHealth();
	void playerUpdate(float dt);
	void shoot();// shoots a bullet (might need delta time if the bullets push the player back)
	void die();
	
	b2Vec2 moveVelocity;
	b2Vec2 jumpVelocity;
	int bulletCount;// amount of bullets the player has
	float health;// health.. pretty self explanitiory, when at 0 the player dies
	b2Body* player_body_;

	std::vector<Bullet> bullets;
};

class Enemy : public GameObject
{
public:
	Enemy();
	void DecrementHealth();
	void enemyUpdate(float dt);
	void shoot();// shoots a bullet (might need delta time if the bullets push the player back)
	void die();

	b2Vec2 moveVelocity;
	b2Vec2 jumpVelocity;
	float health;// health.. pretty self explanitiory, when at 0 the player dies

	std::vector<Bullet> bullets;// the enemy wont have a bullet count as they can shoot infinatly
};



class  Floor: public GameObject
{
public:
	Floor();
	gef::Vector4 halfDimentions;// might not need to keep track of this but never know

	bool isStatic;

};

#endif // _GAME_OBJECT_H