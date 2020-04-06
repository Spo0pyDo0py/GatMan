#include "game_object.h"
#include <system/debug_log.h>
#include <input\keyboard.h>
#include <system/platform.h>
#include <graphics/mesh.h>
//
// UpdateFromSimulation
// 
// Update the transform of this object from a physics rigid body
//
GameObject::~GameObject() {
	inputManP = NULL;
	delete inputManP;

	primitiveBuilderP = NULL;
	delete primitiveBuilderP;

	worldP = NULL;
	delete worldP;

}
void GameObject::UpdateFromSimulation(const b2Body* body)// does the collide
{
	if (body)
	{
		// setup object rotation
		gef::Matrix44 object_rotation;
		object_rotation.RotationZ(body->GetAngle());

		// setup the object translation
		gef::Vector4 object_translation(body->GetPosition().x, body->GetPosition().y, 0.0f);

		// build object transformation matrix
		gef::Matrix44 object_transform = object_rotation;
		object_transform.SetTranslation(object_translation);
		set_transform(object_transform);
	}
}

void GameObject::MyCollisionResponse()
{
	//gef::DebugOut("A collision has happened.\n");
}

/*getters 'n' setters*/
void GameObject::setInputMan(gef::InputManager* inInputMan) {
	inputManP = inInputMan;
}

void GameObject::setPrimitiveBuilder(PrimitiveBuilder* inPrimitiveBuilder) {
	primitiveBuilderP = inPrimitiveBuilder;
}
void GameObject::setWorld(b2World* inWorld) {
	worldP = inWorld;
}

gef::InputManager* GameObject::getInputMan() {
	return inputManP;
}

PrimitiveBuilder* GameObject::getPrimitiveBuilder() {
	return primitiveBuilderP;
}

b2World* GameObject::getWorld() {
	return worldP;
}

#pragma region Player Stuff
Player::Player()
{
	set_type(PLAYER);
	moveVelocity.x = 300;
	moveVelocity.y = 0;

	jumpVelocity.x = 0;
	jumpVelocity.y = 400;


}

void Player::playerInit() {
	set_mesh(primitiveBuilderP->GetDefaultCubeMesh());

	// create a physics body for the player
	b2BodyDef playerBodyDef;
	playerBodyDef.type = b2_dynamicBody;
	playerBodyDef.position = b2Vec2(0.0f, 4.0f);

	playerBody = worldP->CreateBody(&playerBodyDef);

	// create the shape for the player
	b2PolygonShape playerShape;
	playerShape.SetAsBox(0.5f, 0.5f);

	// create the fixture
	b2FixtureDef playerFixtureDef;
	playerFixtureDef.shape = &playerShape;
	playerFixtureDef.density = 1.0f;

	// create the fixture on the rigid body
	playerBody->CreateFixture(&playerFixtureDef);

	// update visuals from simulation data
	UpdateFromSimulation(playerBody);

	// create a connection between the rigid body and GameObject
	//Player tempPlayer = *this;
	//auto* tempP = this;// <--- this is currently yeeting erros at me and I hate itttttttt :(((
	playerBody->SetUserData(this);

	for (int i = 0; i < 10; i++) {
		Bullet newBullet;
		newBullet.setPrimitiveBuilder(getPrimitiveBuilder());
		newBullet.setWorld(getWorld());
		newBullet.bulletInit();
		newBullet.bulletBody->SetActive(0);

		bullets.push_back(newBullet);
	}

	currentBullet = &bullets[0];
}

void Player::DecrementHealth()
{
	//gef::DebugOut("Player has taken damage.\n");
}

void Player::playerUpdate(float dt) {


}

void Player::playerUpdateControls(float dt, gef::Keyboard* keyboard) {
	//gef::Keyboard* keyboard = inputManP->keyboard();// makes a local keyboard for the front end update so it can read keyboard input
	if (inputManP->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_SPACE)) {// makes player jump

		playerBody->ApplyForceToCenter(jumpVelocity, 1);
	}

	if (inputManP->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_RIGHT)) {// makes player move right
		playerBody->SetLinearVelocity(b2Vec2(moveVelocity.x * dt, playerBody->GetLinearVelocity().y));
	}

	if (inputManP->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_LEFT)) {// makes player move left
		playerBody->SetLinearVelocity(b2Vec2(-moveVelocity.x * dt, playerBody->GetLinearVelocity().y));
	}

	if (inputManP->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_D)) {// makes player shoot right
		currentBullet->bulletBody->SetActive(1);
		currentBullet->shoot(b2Vec2(10*dt, 0), gef::Vector4(playerBody->GetPosition().x, playerBody->GetPosition().y, 0));
	}
}



void Player::die() {

}
#pragma endregion

#pragma region Floor Stuff
Floor::Floor()
{
	set_type(FLOOR);
	isStatic = 1;
	halfDimentions = gef::Vector4(1, 1, 1, 0);




}

void Floor::floorInit(gef::Vector4 inHalfDimentions, gef::Vector4 inPosition) {
	// in here could have this to loop i amount of times with random (within reasonable parameters of course) sizes and positions to make a new randomly generated level everytime

// ground dimensions
	halfDimentions = inHalfDimentions;

	// setup the mesh for the ground
	floorMesh = primitiveBuilderP->CreateBoxMesh(halfDimentions);
	set_mesh(floorMesh);

	// create a physics body
	b2BodyDef body_def;
	body_def.type = b2_staticBody;
	body_def.position = b2Vec2(inPosition.x(), inPosition.y());

	floorBody = worldP->CreateBody(&body_def);

	// create the shape
	b2PolygonShape shape;

	shape.SetAsBox(halfDimentions.x(), halfDimentions.y());

	// create the fixture
	b2FixtureDef fixture_def;
	fixture_def.shape = &shape;

	// create the fixture on the rigid body
	floorBody->CreateFixture(&fixture_def);

	// update visuals from simulation data
	UpdateFromSimulation(floorBody);
}


#pragma endregion

#pragma region Enemy Stuff
Enemy::Enemy()
{
	set_type(ENEMY);
	moveVelocity.x = 5;
	moveVelocity.y = 0;

	jumpVelocity.x = 0;
	jumpVelocity.y = 500;


}

void Enemy::DecrementHealth()
{
	//gef::DebugOut("Player has taken damage.\n");
}

void Enemy::enemyUpdate(float dt) {


}

void Enemy::shoot() {

}

void Enemy::die() {

}
#pragma endregion

#pragma region Bullet Stuff
Bullet::Bullet()
{
	set_type(BULLET);
	moveVelocity = gef::Vector4(1, 0, 0, 0);
	damage = 69;
}

void Bullet::bulletInit() {
	set_mesh(primitiveBuilderP->GetDefaultCubeMesh());

	// create a physics body for the player
	b2BodyDef bulletBodyDef;
	bulletBodyDef.type = b2_dynamicBody;
	bulletBodyDef.position = b2Vec2(0.0f, 0.0f);// ment to be in front of player

	bulletBody = worldP->CreateBody(&bulletBodyDef);

	// create the shape for the player
	b2PolygonShape bulletShape;
	bulletShape.SetAsBox(0.1f, 0.2f);

	// create the fixture
	b2FixtureDef bulletFixtureDef;
	bulletFixtureDef.shape = &bulletShape;
	bulletFixtureDef.density = 1.0f;

	// create the fixture on the rigid body
	bulletBody->CreateFixture(&bulletFixtureDef);

	// update visuals from simulation data
	UpdateFromSimulation(bulletBody);

	// create a connection between the rigid body and GameObject
	bulletBody->SetUserData(this);
	bulletBody->SetBullet(1);
	bulletBody->SetAwake(0);
}

void Bullet::shoot(b2Vec2 bulletVelocity, gef::Vector4 bulletPos) {
	bulletBody->SetLinearVelocity(bulletVelocity);

}


void Bullet::bulletUpdate(float dt) {


}

void Bullet::die() {

}


#pragma endregion



