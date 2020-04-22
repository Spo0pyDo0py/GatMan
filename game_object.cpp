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

void GameObject::setScene(gef::Scene* inScene) {
	sceneP = inScene;
}

void GameObject::setPlatform(gef::Platform* inPlatform) {
	platformP = inPlatform;
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

#pragma region Player Stuff
Player::Player()
{
	set_type(PLAYER);
	moveVelocity.x = 300;
	moveVelocity.y = 0;

	jumpVelocity.x = 0;
	jumpVelocity.y = 7;


}

void Player::playerInit() {
	// loads in the model for the player
	halfDimentions = b2Vec2(0.2f, 1.3f);
	//gef::Matrix44 transform;
	//transform.RotationX(-180);
	//transform.SetTranslation(gef::Vector4(halfDimentions.x*2, halfDimentions.y*2, 0, 0));

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
	//playerModel.set_transform(transform);
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

	// create a connection between the rigid body and GameObject
	//Player tempPlayer = *this;
	//auto* tempP = this;// <--- this is currently yeeting erros at me and I hate itttttttt :(((
	playerBody->SetUserData(this);
	playerBody->SetFixedRotation(1);// makes it so the player cant rotate
	/*for (int i = 0; i < 10; i++) {
		Bullet newBullet;
		newBullet.setPrimitiveBuilder(getPrimitiveBuilder());
		newBullet.setWorld(getWorld());
		newBullet.bulletInit();
		newBullet.bulletBody->SetActive(1);

		bullets.push_back(newBullet);
	}*/
	bulletIndex = 0;
	health = 100;
	//currentBullet = &bullets[bulletIndex];

	/* this stuff is for model loading for the player :)	
	
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);

	// create the renderer for draw 3D geometry
	renderer_3d_ = gef::Renderer3D::Create(platform_);

	// initialise primitive builder to make create some 3D geometry easier
	primitive_builder_ = new PrimitiveBuilder(platform_);

	// load the assets in from the .scn
	const char* scene_asset_filename = "world.scn";
	scene_assets_ = LoadSceneAssets(platform_, scene_asset_filename);
	if (scene_assets_)
	{
		mesh_instance_.set_mesh(GetMeshFromSceneAssets(scene_assets_));
	}
	else
	{
		gef::DebugOut("Scene file %s failed to load\n", scene_asset_filename);
	}
	
	// in header
		gef::MeshInstance mesh_instance_;
		gef::Scene* scene_assets_;
	
	
	*/
	isDead = 0;
}

void Player::decrementHealth(float inDamage)
{
	health -= inDamage;
}

void Player::playerUpdate(float dt) {
	if (!isDead) {
		if (playerBody->GetPosition().y <= -10) {
			die();
		}

		if (health <= 0) {
			die();
		}
	}


}

void Player::playerUpdateControls(float dt, gef::Keyboard* keyboard) {
	//gef::Keyboard* keyboard = inputManP->keyboard();// makes a local keyboard for the front end update so it can read keyboard input
	if (!isDead) {
		if (inputManP->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_SPACE)) {// makes player jump
			if (!isJumping) {
				isJumping = 1;
				playerBody->ApplyLinearImpulseToCenter(jumpVelocity, 1);

			}
		}

		if (inputManP->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_RIGHT)) {// makes player move right
			playerBody->SetLinearVelocity(b2Vec2(moveVelocity.x * dt, playerBody->GetLinearVelocity().y));
		}

		if (inputManP->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_LEFT)) {// makes player move left
			playerBody->SetLinearVelocity(b2Vec2(-moveVelocity.x * dt, playerBody->GetLinearVelocity().y));
		}

		if (inputManP->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_D)) {// makes player shoot right
			//currentBullet->bulletBody->SetActive(0);
			//currentBullet->shoot(b2Vec2(10*dt, 0), gef::Vector4(playerBody->GetPosition().x + 1, playerBody->GetPosition().y, 0));
			shoot(b2Vec2(10 * dt, 0), gef::Vector4(playerBody->GetPosition().x +1, playerBody->GetPosition().y + 1, 0));

			// makes sure the current bullet is up to date
			//bulletIndex++;
			//currentBullet = &bullets[bulletIndex];
			//if (bulletIndex == 9)
			//	bulletIndex = 0;
		}
	}
	

}

void Player::shoot(b2Vec2 inBulletVelocity, gef::Vector4 inBulletPos) {
	Bullet* newBullet = new Bullet();
	newBullet->setPrimitiveBuilder(getPrimitiveBuilder());
	newBullet->setWorld(getWorld());
	newBullet->bulletInit(inBulletVelocity, inBulletPos);
	newBullet->bulletBody->SetActive(1);

	bullets.push_back(newBullet);// only in this scope does this bullet exist
}

void Player::die() {
	// commit die on the player
	playerBody->SetFixedRotation(0);
	playerBody->ApplyTorque(10, 1);
	isDead = 1;
	

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

void Enemy::decrementHealth(float damage)
{
	health -= damage;
}

void Enemy::enemyInit(gef::Vector4 inPosition, float inHealth) {
	//gef::Matrix44 transform;
	//transform.RotationY(180);
	//enemyModel.set_transform(transform);
	const char* scene_asset_filename = "enemy.scn";
	sceneP = LoadSceneAssets(*platformP, scene_asset_filename);
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
	/*for (int i = 0; i < 10; i++) {
		Bullet newBullet;
		newBullet.setPrimitiveBuilder(getPrimitiveBuilder());
		newBullet.setWorld(getWorld());
		newBullet.bulletInit();
		newBullet.bulletBody->SetActive(0);

		bullets.push_back(newBullet);
	}

	currentBullet = &bullets[0];*/
	//enemyBody->ApplyTorque(6000.0f, 1); turn on for crazy stuff to happen
	isDead = 0;
}

void Enemy::enemyUpdate(float dt) {
	if (health <= 0) {
		die();
	}
}

void Enemy::shoot() {
	if (!isDead) {

	}
}

void Enemy::die() {
	enemyBody->SetFixedRotation(0);// makes enemy flop over
	enemyBody->ApplyTorque(10, 1);


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
	//srand(time(NULL));
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

	if (rand() % 2 > 0) {
		hasEnemy = 1;
		//platformEnemyP = inEnemyP;
	}
	else hasEnemy = 0;
	//hasEnemy = 1;
}


#pragma endregion

#pragma region Bullet Stuff
Bullet::Bullet()
{
	set_type(BULLET);
	moveVelocity = gef::Vector4(500, 0, 0, 0);
	damage = 69;
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
	//bulletBody->SetAwake(1);
	bulletBody->ApplyLinearImpulseToCenter(bulletVelocity, 1);
}



void Bullet::bulletUpdate(float dt) {


}

void Bullet::die() {// should be called when making contact with an enemy
	bulletBody == NULL;
	delete &bulletBody;
}


#pragma endregion



