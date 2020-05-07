#include "game_object.h"
#include <system/debug_log.h>
#include <input\keyboard.h>
#include <system/platform.h>
#include <graphics/mesh.h>
GameObject::~GameObject() {
	inputManP = NULL;
	delete inputManP;

	primitiveBuilderP = NULL;
	delete primitiveBuilderP;

	worldP = NULL;
	delete worldP;

	platformP = NULL;
	delete platformP;

	sceneP = NULL;
	delete sceneP;


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
gef::Scene* GameObject::LoadSceneAssets(gef::Platform& platform, const char* filename)
{
	gef::Scene* scene = new gef::Scene();

	if (scene->ReadSceneFromFile(platform, filename))
	{
		// if scene file loads successful
		// create material and mesh resources from the scene data
		scene->CreateMaterials(platform);
		scene->CreateMeshes(platform);
	}
	else
	{
		delete scene;
		scene = NULL;
	}

	return scene;
}

gef::Mesh* GameObject::GetMeshFromSceneAssets(gef::Scene* scene){// loads in a mesh from scene 
	gef::Mesh* mesh = NULL;

	// if the scene data contains at least one mesh
	// return the first mesh
	if (scene && scene->meshes.size() > 0)
		mesh = scene->meshes.front();

	return mesh;
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

void GameObject::setScene(gef::Scene* inScene) {
	sceneP = inScene;
}

void GameObject::setPlatform(gef::Platform* inPlatform) {
	platformP = inPlatform;
}
void GameObject::setAudioMan(gef::AudioManager* inAudioMan) {
	audioManP = inAudioMan;
}
gef::Platform* GameObject::getPlatform() {
	return platformP;
}

gef::Scene* GameObject::getScene() {
	return sceneP;
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
#pragma region Goal Stuff
Goal::Goal() {
	set_type(GOAL);
}
void Goal::goalInit(b2Vec2 inPosition) {
	// loads in the model for the goal
	halfDimentions = b2Vec2(1.0f, 1.0f);

	const char* scene_asset_filename = "chungus.scn";// sets goals model to the big chungus model
	sceneP = LoadSceneAssets(*platformP, scene_asset_filename);
	if (sceneP)
	{
		goalModel.set_mesh(GetMeshFromSceneAssets(sceneP));
	}
	else
	{
		gef::DebugOut("Scene file %s failed to load\n", scene_asset_filename);
	}
	//playerModel.set_transform(transform);
	set_mesh(goalModel.mesh());

	// create a physics body for the player
	b2BodyDef goalBodyDef;
	goalBodyDef.type = b2_staticBody;
	goalBodyDef.position = inPosition;

	goalBody = worldP->CreateBody(&goalBodyDef);

	// create the shape for the player
	b2PolygonShape goalShape;
	goalShape.SetAsBox(halfDimentions.x, halfDimentions.y);

	// create the fixture
	b2FixtureDef goalFixtureDef;
	goalFixtureDef.shape = &goalShape;
	goalFixtureDef.density = 1.0f;

	// create the fixture on the rigid body
	goalBody->CreateFixture(&goalFixtureDef);

	// update visuals from simulation data
	UpdateFromSimulation(goalBody);


	goalBody->SetUserData(this);
}
void Goal::goalUpdate(float dt) {

}
#pragma endregion




#pragma region Player Stuff
Player::Player()
{
	set_type(PLAYER);
	moveVelocity.x = 29;
	moveVelocity.y = 0;

	jumpVelocity.x = 0;
	jumpVelocity.y = 230;
}

void Player::playerInit(int32 inGatmanShotSound) {// takes in the gatman shot sound index for the audio man
	halfDimentions = b2Vec2(0.2f, 1.3f);

	// loads in the model for the player
	const char* scene_asset_filename = "gatman.scn";
	sceneP = LoadSceneAssets(*platformP, scene_asset_filename);
	if (sceneP)
	{
		playerModel.set_mesh(GetMeshFromSceneAssets(sceneP));
	}
	else
	{
		gef::DebugOut("Scene file %s failed to load\n", scene_asset_filename);
	}
	set_mesh(playerModel.mesh());
	
	// create a physics body for the player
	b2BodyDef playerBodyDef;
	playerBodyDef.type = b2_dynamicBody;
	playerBodyDef.position = b2Vec2(0.0f, 0.0f);

	playerBody = worldP->CreateBody(&playerBodyDef);

	// create the shape for the player
	b2PolygonShape playerShape;
	playerShape.SetAsBox(halfDimentions.x, halfDimentions.y);

	// create the fixture
	b2FixtureDef playerFixtureDef;
	playerFixtureDef.shape = &playerShape;
	playerFixtureDef.density = 1.0f;

	// create the fixture on the rigid body
	playerBody->CreateFixture(&playerFixtureDef);

	// update visuals from simulation data
	UpdateFromSimulation(playerBody);

	playerBody->SetUserData(this);
	playerBody->SetFixedRotation(1);// makes it so the player cant rotate

	health = 100;
	isDead = 0;

	playerClock.Reset();
	gatmanShotSound = inGatmanShotSound;
}

void Player::decrementHealth(float inDamage)
{
	health -= inDamage;// health isn't even private, but im invested to having this function so it isnt getting deleted
}

void Player::playerUpdate(float dt) {


	if (!isDead) {// so they player doesnt die about 50 times a second if they die
		if (playerBody->GetPosition().y <= -10) {// kills player if they fall off the platforms
			health = -1;
		}

		if (health <= -1) {// kills the player
			die();
		}
	}

	// deleton for bullets
	for (int i = 0; i < bullets.size(); i++) {
		if (bullets[i] == NULL) {

		}
		else {
			if (!bullets[i]->isAlive) {// bullet is null
				delete bullets[i];
				bullets[i] = NULL;
			}
		}
	}

	if (playerClock.GetMilliseconds() >= 700) {// 0.7 second delay between shots for the player
		canShoot = 1;
	}

}

void Player::playerUpdateControls(float dt, gef::Keyboard* keyboard) {
	if (inputManP->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_SPACE)) {// makes player jump
		if (!isJumping) {
			playerBody->ApplyForceToCenter(b2Vec2(jumpVelocity.x, jumpVelocity.y), 1);
		}
		isJumping = 1;
	}
		
	if (inputManP->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_RIGHT)) {// makes player move right
		playerBody->ApplyLinearImpulseToCenter(b2Vec2(moveVelocity.x * dt, playerBody->GetLinearVelocity().y * dt), 1);
	}

	if (inputManP->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_LEFT)) {// makes player move left
		playerBody->ApplyLinearImpulseToCenter(b2Vec2(-moveVelocity.x * dt, playerBody->GetLinearVelocity().y * dt),1);
	}
	if (canShoot) {
		if (inputManP->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_D)) {// makes player shoot right
			shoot(b2Vec2(10 * dt, 0), gef::Vector4(playerBody->GetPosition().x + 1.3, playerBody->GetPosition().y + 1, 0));
			playerClock.Reset();
			canShoot = 0;
			audioManP->PlaySample(gatmanShotSound, 0);
		}
		if (inputManP->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_W)) {// makes player shoot up
			shoot(b2Vec2(0, 10 * dt), gef::Vector4(playerBody->GetPosition().x, playerBody->GetPosition().y + 2.0f, 0));
			playerClock.Reset();
			canShoot = 0;
			audioManP->PlaySample(gatmanShotSound, 0);
		}
		if (inputManP->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_A)) {// makes player shoot back
			shoot(b2Vec2(-10 * dt, 0), gef::Vector4(playerBody->GetPosition().x - 1.3, playerBody->GetPosition().y + 1, 0));
			playerClock.Reset();
			canShoot = 0;
			audioManP->PlaySample(gatmanShotSound, 0);
		}
		if (inputManP->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_S)) {
		if (isJumping) {// if the player is jumping, sends bullet at a downward angle
				shoot(b2Vec2(10 * dt, -5 * dt), gef::Vector4(playerBody->GetPosition().x + 1, playerBody->GetPosition().y, 0));
				playerClock.Reset();
				canShoot = 0;
				audioManP->PlaySample(gatmanShotSound, 0);
			}
			else {// makes player shoot low
				shoot(b2Vec2(10 * dt, 0), gef::Vector4(playerBody->GetPosition().x + 1, playerBody->GetPosition().y, 0));
				playerClock.Reset();
				canShoot = 0;
				audioManP->PlaySample(gatmanShotSound, 0);
			}

		}
	}
}

void Player::shoot(b2Vec2 inBulletVelocity, gef::Vector4 inBulletPos) {
	Bullet* newBullet = new Bullet();// makes a new bullet at in bullet pos which will travel in direction in bullet velocity
	newBullet->setPrimitiveBuilder(getPrimitiveBuilder());
	newBullet->setWorld(getWorld());
	newBullet->bulletInit(inBulletVelocity, inBulletPos);

	for (int i = 0; i < bullets.size(); i++) {
		if (bullets[i] == NULL) {
			bullets[i] = newBullet;
			return;
		}
	}
	bullets.push_back(newBullet);// only in this scope does this bullet exist
} 

void Player::die() {
	// commit die on the player
	playerBody->SetFixedRotation(0);
	playerBody->ApplyTorque(30, 1);// so they flop over
	isDead = 1;
	health = -1;

}
#pragma endregion

#pragma region Enemy Stuff
Enemy::Enemy()
{
	set_type(ENEMY);
	moveVelocity.x = 5;
	moveVelocity.y = 0;

	jumpVelocity.x = 0;
	jumpVelocity.y = 1000;// the enemies were gonna be able to jump on a timer (randomized within range of course) but I ran out of time 
}

void Enemy::decrementHealth(float damage)
{
	health -= damage;
}

void Enemy::enemyInit(gef::Vector4 inPosition, float inHealth, int32 inEnemyShotSound) {
	const char* scene_asset_filename = "enemy.scn";
	sceneP = LoadSceneAssets(*platformP, scene_asset_filename);// loads enemy's model
	if (sceneP)
	{
		enemyModel.set_mesh(GetMeshFromSceneAssets(sceneP));
	}
	else
	{
		gef::DebugOut("Scene file %s failed to load\n", scene_asset_filename);
	}

	set_mesh(enemyModel.mesh());

	// create a physics body for the player
	b2BodyDef enemyBodyDef;
	enemyBodyDef.type = b2_dynamicBody;
	enemyBodyDef.position = b2Vec2(inPosition.x(), inPosition.y()+3);

	enemyBody = worldP->CreateBody(&enemyBodyDef);

	// create the shape for the player
	b2PolygonShape enemyShape;
	enemyShape.SetAsBox(0.2f, 1.3f);

	// create the fixture
	b2FixtureDef enemyFixtureDef;
	enemyFixtureDef.shape = &enemyShape;
	enemyFixtureDef.density = 1.0f;
	
	// create the fixture on the rigid body
	enemyBody->CreateFixture(&enemyFixtureDef);

	// update visuals from simulation data
	UpdateFromSimulation(enemyBody);

	// create a connection between the rigid body and GameObject
	//Player tempPlayer = *this;
	//auto* tempP = this;// <--- this is currently yeeting erros at me and I hate itttttttt :(((
	enemyBody->SetUserData(this);

	enemyBody->SetFixedRotation(1);
	health = inHealth;

	//enemyBody->ApplyForceToCenter(b2Vec2(0,1000), 1);// turn on for crazy stuff to happen
	//enemyBody->ApplyTorque(6000.0f, 1);// turn on for more crazy stuff to happen

	isDead = 0;


	enemyClock.Reset();
	enemyShotSound = inEnemyShotSound;
}

void Enemy::enemyUpdate(float dt, b2Vec2 inPlayerPos) {

	if (!isDead) {
		if (enemyBody->GetPosition().y <= -10) {
			die();
		}

		if (health <= 0) {
			die();
		}
	}

	b2Vec2 playerToEnemy;// vector from enemy to player
	playerToEnemy = inPlayerPos - enemyBody->GetPosition();
	
	if (playerToEnemy.x >= -15.0f) {
		if (enemyClock.GetMilliseconds() >= (3000 / shotDelay)) {
			enemyClock.Reset();

			// start shooting on timer
			if (!isDead) {
				audioManP->PlaySample(enemyShotSound, 0);
				Bullet* newBullet = new Bullet();
				newBullet->setPrimitiveBuilder(getPrimitiveBuilder());
				newBullet->setWorld(getWorld());
				newBullet->bulletInit(b2Vec2(-10 * dt, 0), gef::Vector4(enemyBody->GetPosition().x - 1.3, enemyBody->GetPosition().y + 1, 0));

				for (int i = 0; i < bullets.size(); i++) {
					if (bullets[i] == NULL) {
						bullets[i] = newBullet;
						return;
					}
				}

				bullets.push_back(newBullet);// only in this scope does this bullet exist
			}
		}
	}

	// deleton for bullets
	for (int i = 0; i < bullets.size(); i++) {
		if (bullets[i] == NULL) {

		}
		else {
			if (!bullets[i]->isAlive) {// bullet is null
				delete bullets[i];
				bullets[i] = NULL;
			}
		}
	}

}



void Enemy::die() {
	enemyBody->SetFixedRotation(0);// makes enemy flop over
	enemyBody->ApplyTorque(-30, 1);
	isDead = 1;


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
	// in here could have this to loop i amount of times with random (within reasonable parameters of course) sizes and positions to make a new randomly generated level everytime <<-- also was the first comment here so I refuse to delete it

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

	if (rand() % 2 > 0) {// sometimes an enemy is on a platform
		hasEnemy = 1;
	}
	else hasEnemy = 0;
}


#pragma endregion

#pragma region Bullet Stuff
Bullet::Bullet()
{
	set_type(BULLET);
	moveVelocity = gef::Vector4(0.05, 0.01, 0, 0);
	damage = 49;// so gatman takes 3 bullets to die
	halfSizes = gef::Vector4(0.2f, 0.1f, 0.1f);
}

void Bullet::bulletInit(b2Vec2 bulletVelocity, gef::Vector4 bulletPos) {
	set_mesh(primitiveBuilderP->CreateBoxMesh(halfSizes));

	// create a physics body for the player
	b2BodyDef bulletBodyDef;
	bulletBodyDef.type = b2_dynamicBody;
	bulletBodyDef.position = b2Vec2(bulletPos.x(), bulletPos.y());// ment to be in front of player

	bulletBody = worldP->CreateBody(&bulletBodyDef);

	// create the shape for the player
	b2PolygonShape bulletShape;
	bulletShape.SetAsBox(halfSizes.x(), halfSizes.y());// this should set it to be 

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
	bulletBody->SetBullet(1);// this isn't to do with it crashing
	isAlive = 1;
	moveVelocity.set_x(bulletVelocity.x);
	moveVelocity.set_y(bulletVelocity.y);

	bulletBody->ApplyLinearImpulseToCenter(bulletVelocity, 1);
	bulletClock.Reset();
}

void Bullet::bulletUpdate(float dt) {
	bulletBody->ApplyLinearImpulseToCenter(b2Vec2(moveVelocity.x(), moveVelocity.y()), 1);

	if (bulletClock.GetMilliseconds() >= 700) {// bullets despawn after 2 seconds
		bulletClock.Reset();
		isAlive = 0;
	}
}

void Bullet::die() {// should be called when making contact with an enemy
	bulletBody = NULL;
	delete &bulletBody;
}
#pragma endregion



