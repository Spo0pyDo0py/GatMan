#include "scene_app.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/font.h>
#include <system/debug_log.h>
#include <graphics/renderer_3d.h>
#include <graphics/mesh.h>
#include <maths/math_utils.h>
#include <input/sony_controller_input_manager.h>
#include <input\keyboard.h>
#include <graphics/sprite.h>
#include "load_texture.h"



SceneApp::SceneApp(gef::Platform& platform) :
	Application(platform),
	sprite_renderer_(NULL),
	renderer_3d_(NULL),
	primitive_builder_(NULL),
	input_manager_(NULL),
	font_(NULL),
	world(NULL),
	//player_body_(NULL),
	button_icon_(NULL)
{
}

void SceneApp::Init()
{
	srand(time(NULL));
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	InitFont();

	// initialise input manager
	input_manager_ = gef::InputManager::Create(platform_);

	gameState = LOAD;
	FrontendInit();

}

void SceneApp::CleanUp()
{
	// destroying the physics world also destroys all the objects within it
	delete world;
	world = NULL;

	//delete ground_mesh_;
	//ground_mesh_ = NULL;

	CleanUpFont();

	//audio_manager_->UnloadAllSamples();
	//audio_manager_->UnloadMusic();
//	delete audio_manager_;
//	audio_manager_ = NULL;

	delete input_manager_;
	input_manager_ = NULL;

	delete primitive_builder_;
	primitive_builder_ = NULL;

	delete renderer_3d_;
	renderer_3d_ = NULL;

	delete sprite_renderer_;
	sprite_renderer_ = NULL;
}

bool SceneApp::Update(float frame_time)
{
	fps_ = 1.0f / frame_time;


	input_manager_->Update();


	switch (gameState) {
	case LOAD:
		FrontendUpdate(frame_time);
		break;
	case GAME:
		GameUpdate(frame_time);
		break;
	}


	return true;
}




void SceneApp::Render()
{
	switch (gameState) {
	case LOAD:
		FrontendRender();
	break;
	case GAME:
		GameRender();
	break;
	}

}

void SceneApp::InitPlayer()
{
	// setup the mesh for the player
	player.setInputMan(input_manager_);
	player.setPrimitiveBuilder(primitive_builder_);
	player.setWorld(world);

	player.playerInit();
	
	
	// sets up the cams n that
	freeCam.setPosition(gef::Vector4(player.playerBody->GetPosition().x, player.playerBody->GetPosition().y, 0.0f));
	freeCam.setUp(gef::Vector4(0, 1, 0, 0));
	freeCam.setLookAt(gef::Vector4(player.playerBody->GetPosition().x, player.playerBody->GetPosition().y, 0.0f));

	playerCam.setPosition(gef::Vector4(player.playerBody->GetPosition().x, player.playerBody->GetPosition().y, 0.0f));
	playerCam.setUp(gef::Vector4(0, 1, 0, 0));
	playerCam.setLookAt(gef::Vector4(player.playerBody->GetPosition().x, player.playerBody->GetPosition().y, 0.0f));
}



void SceneApp::InitGround()
{
	ground.setPrimitiveBuilder(primitive_builder_);
	ground.setWorld(world);
	ground.floorInit(gef::Vector4(5.0f, 0.5f, 0.5f), gef::Vector4(0.0f, 0.0f, 0.0f));

	float xIncrement = 8;
	float yIncrement = 0;
	// sets up the vector of platforms and gives them randomized values for their xy position and their length (y)
	for (int i = 0; i < PLATFORM_COUNT; i++) {
		platforms.push_back(new Floor());
		platforms[i]->setPrimitiveBuilder(primitive_builder_);
		platforms[i]->setWorld(world);
		platforms[i]->floorInit(gef::Vector4(2.0f, 0.5f, 0.5f), gef::Vector4(xIncrement, yIncrement, 0.0f));
		xIncrement += rand() % 7 + 3;
		yIncrement += rand() % 3;
		if (platforms[i]->hasEnemy) {
			enemiesCast[i] = 1;// might break the code :(
		}	
	}
}

void SceneApp::InitEnemies() {
	// sets up the vector of enemys and gives them randomized values for their xy position depending on which platform they belong to
	int j = 0;
	for (int i = 0; i < PLATFORM_COUNT; i++) {
		if (enemiesCast[i] == true) {// uses the cast of the pattern in order to see which platforms have enemies. If they do, makes them a thing
			enemies.push_back(new Enemy());
			enemies[j]->setPrimitiveBuilder(primitive_builder_);
			enemies[j]->setWorld(world);
			enemies[j]->enemyInit(gef::Vector4(platforms[i]->floorBody->GetPosition().x, platforms[i]->floorBody->GetPosition().y, 0));
			platforms[i]->platformEnemyP = enemies[j];
			j++;
		}
	}
}



void SceneApp::InitFont()
{
	font_ = new gef::Font(platform_);
	font_->Load("comic_sans");
}

void SceneApp::CleanUpFont()
{
	delete font_;
	font_ = NULL;
}

void SceneApp::DrawHUD()
{
	if(font_)
	{
		// display frame rate
		font_->RenderText(sprite_renderer_, gef::Vector4(850.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "FPS: %.1f", fps_);
	}
}

void SceneApp::SetupLights()
{
	// grab the data for the default shader used for rendering 3D geometry
	gef::Default3DShaderData& default_shader_data = renderer_3d_->default_shader_data();

	// set the ambient light
	default_shader_data.set_ambient_light_colour(gef::Colour(0.25f, 0.25f, 0.25f, 1.0f));

	// add a point light that is almost white, but with a blue tinge
	// the position of the light is set far away so it acts light a directional light
	gef::PointLight default_point_light;
	default_point_light.set_colour(gef::Colour(0.7f, 0.7f, 1.0f, 1.0f));
	default_point_light.set_position(gef::Vector4(-500.0f, 400.0f, 700.0f));
	default_shader_data.AddPointLight(default_point_light);
}

void SceneApp::UpdateSimulation(float frame_time)
{
	// update physics world
	float timeStep = 1.0f / 60.0f;

	int32 velocityIterations = 6;
	int32 positionIterations = 2;

	world->Step(timeStep, velocityIterations, positionIterations);

	// update object visuals from simulation data
	player.UpdateFromSimulation(player.playerBody);
	player.playerUpdate(frame_time);// i added this here just so the player can just have things in an update function rather than putting it all in here although I've kinda had to anyways cos the keyboard is being a pain
	
	for (int i = 0; i < player.bullets.size(); i++) {

		player.bullets[i]->UpdateFromSimulation(player.bullets[i]->bulletBody);
	}

	for (int i = 0; i < enemies.size(); i++) {
		enemies[i]->UpdateFromSimulation(enemies[i]->enemyBody);
	}

	// don't have to update the ground visuals as it is static

	// collision detection
	// get the head of the contact list
	b2Contact* contact = world->GetContactList();
	// get contact count
	int contact_count = world->GetContactCount();

	for (int contact_num = 0; contact_num<contact_count; ++contact_num)
	{
		if (contact->IsTouching())
		{
			// get the colliding bodies
			b2Body* bodyA = contact->GetFixtureA()->GetBody();
			b2Body* bodyB = contact->GetFixtureB()->GetBody();

			// DO COLLISION RESPONSE HERE
			Player* player = NULL;
			Bullet* bullet = NULL;
			Enemy* enemy = NULL;

			GameObject* gameObjectA = NULL;
			GameObject* gameObjectB = NULL;

			gameObjectA = (GameObject*)bodyA->GetUserData();
			gameObjectB = (GameObject*)bodyB->GetUserData();

			if (gameObjectA)
			{
				if (gameObjectA->type() == PLAYER)
				{
					player = (Player*)bodyA->GetUserData();
					player->isJumping = 0;
				}
				if (gameObjectA->type() == ENEMY) {
					enemy = (Enemy*)bodyA->GetUserData();
				}
			}

			if (gameObjectB)
			{
				if (gameObjectB->type() == PLAYER)
				{
					player = (Player*)bodyB->GetUserData();
					//player->isJumping = 0;
				}

				if (gameObjectB->type() == BULLET) {
					bullet = (Bullet*)bodyB->GetUserData();
					float bulletDamage = bullet->damage;
					if (enemy) {
						enemy->decrementHealth(bulletDamage);
					}

				}
			}

			if (player)
			{
				//player->DecrementHealth();
			}
		}

		// Get next contact point
		contact = contact->GetNext();
	}
}

void SceneApp::FrontendInit()
{
	button_icon_ = CreateTextureFromPNG("playstation-cross-dark-icon.png", platform_);
}

void SceneApp::FrontendRelease()
{
	delete button_icon_;
	button_icon_ = NULL;
}

void SceneApp::FrontendUpdate(float frame_time)
{
	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);
	gef::Keyboard* keyboard = input_manager_->keyboard();// makes a local keyboard for the front end update so it can read keyboard input
	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_X)) {
		FrontendRelease();
		gameState = GAME;
		GameInit();
	}

	if (controller->buttons_down() & gef_SONY_CTRL_CROSS) {
		FrontendRelease();
		gameState = GAME;
		GameInit();
	}

}

void SceneApp::FrontendRender()
{
	sprite_renderer_->Begin();

	// render "PRESS" text
	font_->RenderText(
		sprite_renderer_,
		gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
		1.0f,
		0xffffffff,
		gef::TJ_CENTRE,
		"PRESS");

	// Render button icon
	gef::Sprite button;
	button.set_texture(button_icon_);
	button.set_position(gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f, -0.99f));
	button.set_height(32.0f);
	button.set_width(32.0f);
	sprite_renderer_->DrawSprite(button);


	// render "TO START" text
	font_->RenderText(
		sprite_renderer_,
		gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f + 32.0f, -0.99f),
		1.0f,
		0xffffffff,
		gef::TJ_CENTRE,
		"TO START");


	DrawHUD();
	sprite_renderer_->End();
}

void SceneApp::GameInit()
{
	// sets up free cam at default position
	whatCam = 1;

	freeCam.setPosition(gef::Vector4(-5.0f, -20.0f, 5.0f));
	freeCam.setLookAt(gef::Vector4(0.0f, 0.0f, 0.0f));

	// create the renderer for draw 3D geometry
	renderer_3d_ = gef::Renderer3D::Create(platform_);

	// initialise primitive builder to make create some 3D geometry easier
	primitive_builder_ = new PrimitiveBuilder(platform_);


	SetupLights();

	// initialise the physics world
	b2Vec2 gravity(0.0f, -9.81f);
	world = new b2World(gravity);
	enemyCount = 0;
	for (int i = 0; i < PLATFORM_COUNT; i++)// initilizes the enemy randomized pattern cast
		enemiesCast.push_back(0);

	//audio_manager_ = gef::AudioManager::Create();
	//soundBoxCollected = audio_manager_->LoadSample("box_collected.wav", platform_);
	//audio_manager_->LoadMusic("music.wav", platform_);


	InitPlayer();
	InitGround();
	InitEnemies();
}

void SceneApp::GameRelease()
{
	// destroying the physics world also destroys all the objects within it
	delete world;
	world = NULL;

	delete primitive_builder_;
	primitive_builder_ = NULL;

	delete renderer_3d_;
	renderer_3d_ = NULL;

}

void SceneApp::GameUpdate(float frame_time)
{
	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);
	
	
	UpdateSimulation(frame_time);

	gef::Keyboard* keyboard = input_manager_->keyboard();// makes a local keyboard for the front end update so it can read keyboard input
	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_X)) {
		GameRelease();
		gameState = LOAD;
		FrontendInit();
	}
#pragma region Switch Cams
	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_0)) {// freecam
		whatCam = 0;
	}

	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_1)) {// playercam
		whatCam = 1;
	}

	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_2)) {// freezecam
		whatCam = 2;
	}
#pragma endregion


#pragma region Player Controls
	if (whatCam != 0) {

		player.playerUpdateControls(frame_time, keyboard);

	}
	
#pragma endregion

	

#pragma region Freecam Controls
	if (whatCam == 0) {
		// move controls
		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_W)) {// makes freecam move forward
			freeCam.moveForward(frame_time);
		}
		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_S)) {// makes freecam move right
			freeCam.moveBack(frame_time);
		}
		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_A)) {// makes freecam move right
			freeCam.moveLeft(frame_time);
		}
		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_D)) {// makes freecam move right
			freeCam.moveRight(frame_time);
		}

		// rot controls
		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_UP)) {// makes freecam rotate up
			freeCam.rotUp(frame_time);
		}

		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_DOWN)) {// makes freecam rotate down
			freeCam.rotDown(frame_time);
		}

		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_LEFT)) {// makes freecam rotate left
			freeCam.rotLeft(frame_time);
		}

		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_RIGHT)) {// makes freecam rotate right
			freeCam.rotRight(frame_time);
		}
	}

	playerCam.setPosition(gef::Vector4(player.playerBody->GetPosition().x + 2.0f, player.playerBody->GetPosition().y + 2.0f, 10.0f));// <-- this does infact work, 
	playerCam.setLookAt(gef::Vector4(player.playerBody->GetPosition().x, player.playerBody->GetPosition().y, 0.0f));// its just the camera is still only looking up rather than at the box :/
	
#pragma endregion
	
	


	b2Vec2 tempVelo = player.playerBody->GetLinearVelocity();
	tempVelo.x *= 0.95;// slows the box's velocity in the x axis down over time so its not slipping off the platforms
	player.playerBody->SetLinearVelocity(tempVelo);

	/*if (controller->buttons_down() & gef_SONY_CTRL_CIRCLE) {
		GameRelease();
		gameState = LOAD;
		FrontendInit();

	}*/
	// updates cameras
	playerCam.update();
	playerCam.updateLookAt();

	freeCam.update();
	freeCam.updateLookAt();

	// updates enemys
	for (int i = 0; i < enemies.size(); i++) {
		enemies[i]->enemyUpdate(frame_time);
	}

	// updates bullets
	for (int i = 0; i < player.bullets.size(); i++) {
		player.bullets[i]->bulletUpdate(frame_time);
	}


}

void SceneApp::GameRender()
{
	// setup camera

	// projection
	float fov = gef::DegToRad(45.0f);
	float aspect_ratio = (float)platform_.width() / (float)platform_.height();
	gef::Matrix44 projection_matrix;
	projection_matrix = platform_.PerspectiveProjectionFov(fov, aspect_ratio, 0.1f, 100.0f);
	renderer_3d_->set_projection_matrix(projection_matrix);

	// changes the view depending on what cam is being currently used
	gef::Matrix44 view_matrix;
	switch (whatCam) {
	case 0: {	view_matrix.LookAt(freeCam.getPosition(), freeCam.getLookAt(), freeCam.getUp());
		renderer_3d_->set_view_matrix(view_matrix); };
		break;

	case 1: {	view_matrix.LookAt(playerCam.getPosition(), playerCam.getLookAt(), playerCam.getUp());
		renderer_3d_->set_view_matrix(view_matrix); };
		break;

	}

	// draw 3d geometry
	renderer_3d_->Begin();

	// draw ground
	// note for future me: having an array of platforms and just looping through them all here:
	renderer_3d_->DrawMesh(ground);
	for (int i = 0; i < platforms.size(); i++) {
		renderer_3d_->DrawMesh(*platforms[i]);
	}
	for (int i = 0; i < enemies.size(); i++) {
		renderer_3d_->set_override_material(&primitive_builder_->red_material());
		renderer_3d_->DrawMesh(*enemies[i]);
	}

	for (int i = 0; i < player.bullets.size(); i++) {
		renderer_3d_->set_override_material(&primitive_builder_->green_material());
		//if(player.bullets[i]->bulletBody->IsActive())// this is where its crashing
		renderer_3d_->DrawMesh(*player.bullets[i]);// crashes here :(
	}
	


	// draw player
	renderer_3d_->set_override_material(&primitive_builder_->blue_material());
	renderer_3d_->DrawMesh(player);
	renderer_3d_->set_override_material(NULL);

	renderer_3d_->End();

	// start drawing sprites, but don't clear the frame buffer
	sprite_renderer_->Begin(false);
	DrawHUD();
	sprite_renderer_->End();
}
// controls the cameras rotation with the mouse



