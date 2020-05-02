#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include <graphics/mesh_instance.h>
#include <box2d/Box2D.h>
#include <input/input_manager.h>
#include "primitive_builder.h"
#include <vector>
#include "graphics/scene.h"
#include "box2d/b2_body.h"
#include <iostream>
#include <time.h>


enum OBJECT_TYPE
{
	PLAYER,
	ENEMY,
	BULLET,
	FLOOR,
	GOAL
};

class GameObject : public gef::MeshInstance
{
public:
	~GameObject();
	void UpdateFromSimulation(const b2Body* body);
	void MyCollisionResponse();
	gef::Scene* LoadSceneAssets(gef::Platform& platform, const char* filename);
	gef::Mesh* GetMeshFromSceneAssets(gef::Scene* scene);



	/*Getters 'n' Setters*/
	void setInputMan(gef::InputManager* inInputMan);
	void setPrimitiveBuilder(PrimitiveBuilder* inPrimitiveBuilder);
	void setWorld(b2World* inWorld);
	void setScene(gef::Scene* inScene);
	void setPlatform(gef::Platform* inPlatform);
	gef::Platform* getPlatform();
	gef::Scene* getScene();
	gef::InputManager* getInputMan();
	PrimitiveBuilder* getPrimitiveBuilder();
	b2World* getWorld();

	inline void set_type(OBJECT_TYPE type) { type_ = type; }
	inline OBJECT_TYPE type() { return type_; }

	gef::InputManager* inputManP;
	PrimitiveBuilder* primitiveBuilderP;
	b2World* worldP;
	gef::Scene* sceneP;
	gef::Platform* platformP;

private:
	OBJECT_TYPE type_;

};

class Goal : public GameObject
{
public:
	Goal();
	void goalInit(b2Vec2 inPosition);
	void goalUpdate(float dt);
	gef::MeshInstance goalModel;
	b2Vec2 halfDimentions;



	gef::Vector4 halfSizes;// needed for mesh
	b2Body* goalBody;
	bool isAlive;

};

class Bullet : public GameObject
{
public:
	Bullet();
	void bulletInit(b2Vec2 bulletVelocity, gef::Vector4 bulletPos);
	void bulletUpdate(float dt);
	void die();// despawns the bullet



	b2Timer bulletClock;
	gef::Vector4 moveVelocity;// the bullet can miss by going off to the side hence it's a vector 4
	gef::Vector4 halfSizes;// needed for mesh
	float damage;// the damage a bullet does on impact to a gameobject
	b2Body* bulletBody;
	bool isAlive;

};

class Player : public GameObject
{
public:
	Player();
	//~Player();
	void playerInit();
	void decrementHealth(float inDamage);
	void playerUpdate(float dt);
	void playerUpdateControls(float dt, gef::Keyboard* keyboard);
	void shoot(b2Vec2 bulletVelocity, gef::Vector4 inBulletPos);
	void die();

	b2Vec2 moveVelocity;
	b2Vec2 halfDimentions;
	b2Vec2 jumpVelocity;

	//int bulletIndex;// keeps track of the index of the bullets array its on
	float health;// health, keeps track of how much damage the player has taker
	b2Body* playerBody;
	gef::MeshInstance playerModel;
	bool isJumping;
	bool isDead;
	bool onPlatform;
	bool canShoot;

	b2Timer playerClock;

	std::vector<Bullet*> bullets;

};


class Enemy : public GameObject
{
public:
	Enemy();
	//~Enemy();
	void decrementHealth(float damage);
	void enemyInit(gef::Vector4 inPosition, float inHealth);
	void enemyUpdate(float dt, b2Vec2 inPlayerPos);
	void die();

	b2Vec2 moveVelocity;
	b2Vec2 jumpVelocity;
	b2Timer enemyClock;
	float health;// health... when at 0 the enemy dies
	b2Body* enemyBody;
	gef::MeshInstance enemyModel;
	bool isDead;

	std::vector<Bullet*> bullets;
	//Bullet* currentBullet; // points at the current bullet in the mag
};



class  Floor: public GameObject
{
public:
	Floor();
	//~Floor();
	void floorInit(gef::Vector4 inHalfDimentions, gef::Vector4 inPosition);


	gef::Vector4 halfDimentions;// might not need to keep track of this but never know
	b2Body* floorBody;
	gef::Mesh* floorMesh;
	bool isStatic;
	bool hasEnemy;
	Enemy* platformEnemyP;


};

#endif // _GAME_OBJECT_H