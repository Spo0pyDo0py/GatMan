#include "game_object.h"
#include <system/debug_log.h>

//
// UpdateFromSimulation
// 
// Update the transform of this object from a physics rigid body
//
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
void GameObject::setInputMan(gef::InputManager* inInputMan) {
	inputManP = inInputMan;
}

void GameObject::setPrimitiveBuilder(PrimitiveBuilder* inPrimitiveBuilder) {
	primitiveBuilderP = inPrimitiveBuilder;
}
void GameObject::setWorld(b2World* inWorld) {
	worldP = inWorld;
}

#pragma region Player Stuff
Player::Player()
{
	set_type(PLAYER);
	moveVelocity.x = 5;
	moveVelocity.y = 0;

	jumpVelocity.x = 0;
	jumpVelocity.y = 500;


}

void Player::playerInit() {
	set_mesh(primitiveBuilderP->GetDefaultCubeMesh());

	// create a physics body for the player
	b2BodyDef player_body_def;
	player_body_def.type = b2_dynamicBody;
	player_body_def.position = b2Vec2(0.0f, 4.0f);

	player_body_ = worldP->CreateBody(&player_body_def);

	// create the shape for the player
	b2PolygonShape player_shape;
	player_shape.SetAsBox(0.5f, 0.5f);

	// create the fixture
	b2FixtureDef player_fixture_def;
	player_fixture_def.shape = &player_shape;
	player_fixture_def.density = 1.0f;

	// create the fixture on the rigid body
	player_body_->CreateFixture(&player_fixture_def);

	// update visuals from simulation data
	UpdateFromSimulation(player_body_);

	// create a connection between the rigid body and GameObject
	//Player tempPlayer = *this;
	auto* tempP = this;// <--- this is currently yeeting erros at me and I hate itttttttt :(((
	player_body_->SetUserData(&tempP);
}

void Player::DecrementHealth()
{
	//gef::DebugOut("Player has taken damage.\n");
}

void Player::playerUpdate(float dt) {


}

void Player::shoot() {

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

void Bullet::bulletUpdate(float dt) {


}

void Bullet::die() {

}

#pragma endregion



