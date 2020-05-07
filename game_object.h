#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include <graphics/mesh_instance.h>
#include <box2d/Box2D.h>
#include <input/input_manager.h>
#include "primitive_builder.h"
#include <vector>
#include "graphics/scene.h"
#include "box2d/b2_body.h"
#include "audio/audio_manager.h"
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

	gef::Scene* LoadSceneAssets(gef::Platform& platform, const char* filename);
	gef::Mesh* GetMeshFromSceneAssets(gef::Scene* scene);

	/*Getters 'n' Setters*/
	void setInputMan(gef::InputManager* inInputMan);
	void setPrimitiveBuilder(PrimitiveBuilder* inPrimitiveBuilder);
	void setWorld(b2World* inWorld);
	void setScene(gef::Scene* inScene);
	void setPlatform(gef::Platform* inPlatform);
	void setAudioMan(gef::AudioManager* inAudioMan);
	gef::Platform* getPlatform();
	gef::Scene* getScene();
	gef::InputManager* getInputMan();
	PrimitiveBuilder* getPrimitiveBuilder();
	b2World* getWorld();

	inline void set_type(OBJECT_TYPE type) { type_ = type; }
	inline OBJECT_TYPE type() { return type_; }

	// pointers for world stuff
	gef::InputManager* inputManP;
	PrimitiveBuilder* primitiveBuilderP;
	b2World* worldP;
	gef::Scene* sceneP;
	gef::Platform* platformP;
	gef::AudioManager* audioManP;

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
	void playerInit(int32 inplayerShotSound);
	void decrementHealth(float inDamage);
	void playerUpdate(float dt);
	void playerUpdateControls(float dt, gef::Keyboard* keyboard);
	void shoot(b2Vec2 bulletVelocity, gef::Vector4 inBulletPos);
	void die();

	b2Vec2 moveVelocity;
	b2Vec2 halfDimentions;
	b2Vec2 jumpVelocity;

	// audio voice id for gatman's gun shot that is set in init
	int32 gatmanShotSound;

	float health;// health, keeps track of how much damage the player has taken
	b2Body* playerBody;
	gef::MeshInstance playerModel;
	bool isJumping;
	bool isDead;
	bool canShoot;

	b2Timer playerClock;// needed for delay between shots 

	std::vector<Bullet*> bullets;// player's shot bullets

};


class Enemy : public GameObject
{
public:
	Enemy();
	void decrementHealth(float damage);
	void enemyInit(gef::Vector4 inPosition, float inHealth, int32 inEnemyShotSound);
	void enemyUpdate(float dt, b2Vec2 inPlayerPos);// takes in th eposition of the player
	void die();

	b2Vec2 moveVelocity;
	b2Vec2 jumpVelocity;
	b2Timer enemyClock;
	int32 enemyShotSound;	
	float health;// health... when at 0 the enemy dies
	b2Body* enemyBody;
	gef::MeshInstance enemyModel;
	bool isDead;
	float shotDelay;

	std::vector<Bullet*> bullets;
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