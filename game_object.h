#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include <graphics/mesh_instance.h>
#include <box2d/Box2D.h>
#include <input/input_manager.h>
#include "primitive_builder.h"
#include <vector>
#include "box2d/b2_body.h"
#include <iostream>
#include <time.h>

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
	~GameObject();
	void UpdateFromSimulation(const b2Body* body);
	void MyCollisionResponse();



	/*Getters 'n' Setters*/
	void setInputMan(gef::InputManager* inInputMan);
	void setPrimitiveBuilder(PrimitiveBuilder* inPrimitiveBuilder);
	void setWorld(b2World* inWorld);
	gef::InputManager* getInputMan();
	PrimitiveBuilder* getPrimitiveBuilder();
	b2World* getWorld();

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
	void bulletInit(b2Vec2 bulletVelocity, gef::Vector4 bulletPos);
	void bulletUpdate(float dt);
	void die();// despawns the bullet

	/*getters 'n' setters*/



	gef::Vector4 moveVelocity;// the bullet can miss by going off to the side hence it's a vector 4
	gef::Vector4 halfSizes;// needed for mesh
	float damage;// the damage a bullet does on impact to a gameobject
	b2Body* bulletBody;

};

class Player : public GameObject
{
public:
	Player();
	void playerInit();
	void DecrementHealth();
	void playerUpdate(float dt);
	void playerUpdateControls(float dt, gef::Keyboard* keyboard);
	void shoot(b2Vec2 bulletVelocity, gef::Vector4 inBulletPos);
	void die();

	b2Vec2 moveVelocity;
	b2Vec2 jumpVelocity;
	int bulletIndex;// keeps track of the index of the bullets array its on
	float health;// health.. pretty self explanitiory, when at 0 the player dies
	b2Body* playerBody;
	bool isJumping;

	std::vector<Bullet*> bullets;

};


class Enemy : public GameObject
{
public:
	Enemy();
	void decrementHealth(float damage);
	void enemyInit(gef::Vector4 inPosition);
	void enemyUpdate(float dt);
	void shoot();// shoots a bullet (might need delta time if the bullets push the player back)
	void die();

	b2Vec2 moveVelocity;
	b2Vec2 jumpVelocity;
	int bulletCount;// amount of bullets the player has
	float health;// health.. pretty self explanitiory, when at 0 the player dies
	b2Body* enemyBody;

	std::vector<Bullet> bullets;
	//Bullet* currentBullet; // points at the current bullet in the mag
};



class  Floor: public GameObject
{
public:
	Floor();
	void floorInit(gef::Vector4 inHalfDimentions, gef::Vector4 inPosition);


	gef::Vector4 halfDimentions;// might not need to keep track of this but never know
	b2Body* floorBody;
	gef::Mesh* floorMesh;
	bool isStatic;
	bool hasEnemy;
	Enemy* platformEnemyP;


};

#endif // _GAME_OBJECT_H