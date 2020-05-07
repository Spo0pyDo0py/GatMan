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
	button_icon_(NULL)
{
}

void SceneApp::Init()
{
	srand(time(NULL));// makes things random
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	audio_manager_ = gef::AudioManager::Create();
	InitFont();
	input_manager_ = gef::InputManager::Create(platform_);
	scene_assets_ = NULL;

	gameState = INTRO;
	IntroInit();

	// declarations for progress flags and difficulty
	difficulty = 1;
	easyBeat = 0;
	easyAced = 0;
	regularBeat = 0;
	regularAced = 0;
	hardBeat = 0;
	hardAced = 0;

	// music and sound forward init's
	musicVolText = 5, SFXVolText = 5, masterVolText = 5;
	changingMusicVol = 0, changingSFXVol = 0, changingMasterVol = 0;
	SFXVolumeInfo.volume = 100;// init for sfx vol info

	//difficulty = 1;

}

void SceneApp::CleanUp()
{
	// destroying the physics world also destroys all the objects within it
	delete world;
	world = NULL;


	CleanUpFont();

	audio_manager_->UnloadAllSamples();
	audio_manager_->UnloadMusic();

	delete audio_manager_;
	audio_manager_ = NULL;

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
	case INTRO:
		IntroUpdate(frame_time);
		break;
	case LOAD:
		FrontendUpdate(frame_time);
		break;
		
	case GAME:
		GameUpdate(frame_time);
		break;

	case DEAD:
		GameUpdate(frame_time);// so you can see the lifeless husk of gatman flop to the floor in the background of death scene :)
		DeadUpdate(frame_time);
		break;

	case WIN:
		WinUpdate(frame_time);
		break;

	case PAUSE:
		PauseUpdate(frame_time);
		break;
	}


	return true;
}




void SceneApp::Render()
{
	switch (gameState) {
	case INTRO:
		IntroRender();
		break;
	case LOAD:
		FrontendRender();
		break;
	case GAME:
		GameRender();
		break;
	case DEAD:
		GameRender();// shows the stuff thats going on in the background #1
		DeadRender();
		break;

	case WIN:
		GameRender();// shows the stuff thats going on in the background #2: the electric boogaloo
		WinRender();
		break;

	case PAUSE:
		GameRender();// shows the stuff thats going on in the background #3.5: pathfinder
		PauseRender();
		break;
	}



}

void SceneApp::InitPlayer(){// sets up enerything to do with the player
	player = new Player();
	// player handles all the keyboard input to do with the player in it's clas along with all the model loading stuff, so it needs pointers to all of these things
	player->setInputMan(input_manager_);
	player->setPrimitiveBuilder(primitive_builder_);
	player->setWorld(world);
	player->setAudioMan(audio_manager_);
	// these two needed for model stuff
	player->setScene(scene_assets_);
	player->setPlatform(&platform_);

	player->playerInit(gatmanShotSound);
	
	
	// sets up the cams
	freeCam->setPosition(gef::Vector4(player->playerBody->GetPosition().x, player->playerBody->GetPosition().y + 2.0f, 7.0f));// sets the free cam up so it spawns in front of the player and looking at them
	freeCam->setUp(gef::Vector4(0, 1, 0, 0));
	freeCam->setLookAt(gef::Vector4(player->playerBody->GetPosition().x, player->playerBody->GetPosition().y, 0.0f));

	playerCam->setPosition(gef::Vector4(player->playerBody->GetPosition().x, player->playerBody->GetPosition().y, 0.0f));// players cam is set up looking at the player
	playerCam->setUp(gef::Vector4(0, 1, 0, 0));
	playerCam->setLookAt(gef::Vector4(player->playerBody->GetPosition().x, player->playerBody->GetPosition().y, 0.0f));
}




void SceneApp::InitGround()
{
	// sets up the first platform so an enemy isn't gonna spawn on it
	ground = new Floor();
	ground->setPrimitiveBuilder(primitive_builder_);
	ground->setWorld(world);
	ground->floorInit(gef::Vector4(5.0f, 0.5f, 0.5f), gef::Vector4(0.0f, 0.0f, 0.0f));// the floor's take in their init; their half dimentions and their position

	float xIncrement = 8;
	float yIncrement = 0;
	// sets up the vector of platforms and gives them randomized values for their xy position and their length (y)
	for (int i = 0; i < platformCount; i++) {
		platforms.push_back(new Floor());
		platforms[i]->setPrimitiveBuilder(primitive_builder_);
		platforms[i]->setWorld(world);
		platforms[i]->floorInit(gef::Vector4(2.0f, 0.5f, 0.5f), gef::Vector4(xIncrement, yIncrement, 0.0f));// all platforms will be the same size but the position is based off the x and y increment and what number gets added to it
		xIncrement += rand() % 7 + 3;// each platform should always be at leats 3 units away on the x axis and will be inbetween 3-10 units away in th ex axis from the last
		yIncrement += rand() % 3;// each platform will be 0-3 units in the y axis away from th elast
		if (platforms[i]->hasEnemy) {// if the platform has an enemy on it, let the cast know so it can be used for the enemy init
			enemiesCast[i] = 1;
		}	
	}



}

void SceneApp::InitEnemies() {
	// sets up the vector of enemys and gives them randomized values for their xy position depending on which platform they belong to
	int j = 0;
	for (int i = 0; i < platformCount; i++) {
		if (enemiesCast[i] == true) {// uses the cast of the pattern in order to see which platforms have enemies. If they do, makes them a thing
			enemies.push_back(new Enemy());// the enemies need pointers to all of these as they load their own models and activate their own sounds
			enemies[j]->setPrimitiveBuilder(primitive_builder_);
			enemies[j]->setWorld(world);
			enemies[j]->setScene(scene_assets_);
			enemies[j]->setPlatform(&platform_);
			enemies[j]->setAudioMan(audio_manager_);

			// the time inbetween shots is smaller depending on which difficulty is selected
			enemies[j]->shotDelay = difficulty;
			enemies[j]->shotDelay *= rand() % 100 + 50;
			enemies[j]->shotDelay /= 100;

			enemies[j]->enemyInit(gef::Vector4(platforms[i]->floorBody->GetPosition().x, platforms[i]->floorBody->GetPosition().y, 0), rand() % 100 + enemyHealthMod, enemyShotSound);// declares enemies at randomized platforms location with a partily randomised amount of health (also dependant on the difficulty)
			platforms[i]->platformEnemyP = enemies[j];
			j++;
		}
	}
}

void SceneApp::InitGoal() {
	// initilises the goal
	goal = new Goal();
	goal->setPrimitiveBuilder(primitive_builder_);
	goal->setWorld(world);
	goal->setScene(scene_assets_);
	goal->setPlatform(&platform_);
	goal->goalInit(b2Vec2(platforms.back()->floorBody->GetPosition().x + 1, platforms.back()->floorBody->GetPosition().y + 2));// sets position of goal just above the last platform

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
		font_->RenderText(sprite_renderer_, gef::Vector4(850.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "FPS: %0.1f", fps_);
		// display player's health in the game or if they're dead
		if (gameState == GAME || gameState == DEAD) {
			if (player->health > 0) {
				font_->RenderText(sprite_renderer_, gef::Vector4(740.0f, 460.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Player's Health: %1.0f", player->health);
			}
			else {
				font_->RenderText(sprite_renderer_, gef::Vector4(885.0f, 460.0f, -0.9f), 1.0f, 0xff0000ff, gef::TJ_LEFT, "ded :(");
			}
			
		}
		
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
	default_point_light.set_colour(gef::Colour(0.3f, 0.3f, 0.5f, 1.0f));// was:   0.1f, 0.1f, 0.3f, 1.0f
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
	player->UpdateFromSimulation(player->playerBody);
	player->playerUpdate(frame_time);// i added this here just so the player can just have things in an update function rather than putting it all in here although I've kinda had to anyways cos the keyboard is being a pain
	

	for (int i = 0; i < player->bullets.size(); i++) {
		if (player->bullets[i] == NULL) {

		}
		else
			player->bullets[i]->UpdateFromSimulation(player->bullets[i]->bulletBody);
	}

		// renders enemy's bullets
	for (int i = 0; i < enemies.size(); i++) {
		for (int j = 0; j < enemies[i]->bullets.size(); j++) {
			if (enemies[i]->bullets[j] == NULL) {

			}
			else {
				enemies[i]->bullets[j]->UpdateFromSimulation(enemies[i]->bullets[j]->bulletBody);
			}
		}
		
	}

	

	for (int i = 0; i < enemies.size(); i++) {// updates enemies physics
		enemies[i]->UpdateFromSimulation(enemies[i]->enemyBody);
	}

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
			Player* playerP = NULL;
			Bullet* bulletP = NULL;
			Enemy* enemyP = NULL;
			Floor* platformP = NULL;
			Goal* goalP = NULL;

			GameObject* gameObjectA = NULL;
			GameObject* gameObjectB = NULL;

			gameObjectA = (GameObject*)bodyA->GetUserData();
			gameObjectB = (GameObject*)bodyB->GetUserData();

			if (gameObjectA)
			{
				if (gameObjectA->type() == PLAYER)
				{
					playerP = (Player*)bodyA->GetUserData();
					playerP->isJumping = 0;

				}
				if (gameObjectA->type() == ENEMY) {
					enemyP = (Enemy*)bodyA->GetUserData();
				}
				if (gameObjectA->type() == FLOOR) {
					platformP = (Floor*)bodyA->GetUserData();
				}

				if (gameObjectA->type() == BULLET) {
					bulletP = (Bullet*)bodyA->GetUserData();
				}

				if (gameObjectA->type() == GOAL) {
					goalP = (Goal*)bodyA->GetUserData();
				}

				if (gameObjectB)
				{
					if (gameObjectB->type() == PLAYER)
					{
						playerP = (Player*)bodyB->GetUserData();

					}

					if (gameObjectB->type() == BULLET) {
						bulletP = (Bullet*)bodyB->GetUserData();
						float bulletDamage = bulletP->damage;
						if (enemyP) {// damages and knocks back an enemy that makes contact with a bullet
							enemyP->decrementHealth(bulletDamage);
							enemyP->enemyBody->ApplyForceToCenter(b2Vec2(30, 15), 1);// knocks back the enemy
							audio_manager_->PlaySample(hitmarkerSound, 0);
							if (bulletP) {
								bulletP->isAlive = 0;
							}
						}
						else if (playerP) {// damages and knocks back a player that makes contact with a bullet
							playerP->playerBody->ApplyForceToCenter(b2Vec2(-30, 15), 1);// knocks back the player
							playerP->decrementHealth(bulletDamage);
							audio_manager_->PlaySample(hitmarkerSound, 0);
							if (bulletP) {
								bulletP->isAlive = 0;
							}

						}						
					}

					if (gameObjectB->type() == ENEMY) {// if  the player touches an enemy they take damage
						enemyP = (Enemy*)bodyB->GetUserData();
						if (enemyP) {
							if (enemyP->isDead) {

							}
							else if (playerP) {
 								playerP->decrementHealth(5);
							}
						}
					}

					if (gameObjectB->type() == FLOOR) {
						platformP = (Floor*)bodyB->GetUserData();
					}

					if (gameObjectB->type() == GOAL) {// if the player touches the goal, they win
						goalP = (Goal*)bodyB->GetUserData();
						if (playerP) {
							WinInit();
							gameState = WIN;
							audio_manager_->PlaySample(winSound, 0);
						}
					}


				}
			}
		}
		// Get next contact point
		contact = contact->GetNext();
	}
}

void SceneApp::IntroInit() {
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	// my collection of amazing quotes
	quotes.push_back("'With lapis and magenta iridescence, the camera with wings takes flight.' - pigeon dude");
	quotes.push_back("'I'm tellin' you man, pigeons are drones.' - pigeon dude (again)");
	quotes.push_back("'Roses are red, my name is Dave. I suck at poetry, microwave.' - Dave");
	quotes.push_back("'Lifes a party, and I'm the pinata' - u/spahgetticheerios");
	quotes.push_back("'Graphic design is my passion' - a cool fella ");
	quotes.push_back("'Roll inititive.' - the DM");

	background = CreateTextureFromPNG("background.png", platform_);
	whatQuote = rand() % 6;// picks what quote is being used
	introSound = audio_manager_->LoadSample("gatman intro.wav", platform_);
	// Start timing
	audio_manager_->PlaySample(introSound, 0);// plays intro music (yes made my me)
	timer.Reset();
}
void SceneApp::IntroRelease() {
	delete sprite_renderer_;
	sprite_renderer_ = NULL;

	delete background;
	background = NULL;

}
void SceneApp::IntroUpdate(float frame_time) {
	if (timer.GetMilliseconds() >= 8000) {// after 8 seconds, go to the main menu
		IntroRelease();
		FrontendInit();
		gameState = LOAD;
	}
}
void SceneApp::IntroRender() {
	sprite_renderer_->Begin();

	gef::Sprite backgroundSprite;
	backgroundSprite.set_texture(background);
	backgroundSprite.set_position(gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f, -0.99f));// background will always be as big as the screen
	backgroundSprite.set_height(platform_.height());
	backgroundSprite.set_width(platform_.width());
	sprite_renderer_->DrawSprite(backgroundSprite);

	font_->RenderText(
		sprite_renderer_,
		gef::Vector4(platform_.width()*0.5f, platform_.height()*0.75f, -0.99f),
		1.0f,
		0xffffffff,
		gef::TJ_CENTRE,
		quotes[whatQuote].c_str());// displays the quote (as an array of characters)
	sprite_renderer_->End();
}


void SceneApp::FrontendInit()
{
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	button_icon_ = CreateTextureFromPNG("playstation-cross-dark-icon.png", platform_);
	backgroundMenu = CreateTextureFromPNG("backgroundMenu.png", platform_);
	clearHeart = CreateTextureFromPNG("u cleared.png", platform_);
	aceHeart = CreateTextureFromPNG("u madlad.png", platform_);

	platform_.set_render_target_clear_colour(gef::Colour(0.5f, 0.8f, 0.5f));// rgba




	audio_manager_->LoadMusic("gatman main menu music.wav", platform_);// initiates the banging beats
	audio_manager_->PlayMusic();// plays the banging beats

	switch (difficulty) {
	case 1:	platform_.set_render_target_clear_colour(gef::Colour(0.5f, 0.8f, 0.5f));// rgba
		break;

	case 2:	platform_.set_render_target_clear_colour(gef::Colour(0.3f, 0.3f, 1.0f));// rgba
		break;

	case 3:	platform_.set_render_target_clear_colour(gef::Colour(0, 0, 0));// rgba
		break;

	}
}

void SceneApp::FrontendRelease()
{
	delete button_icon_;
	button_icon_ = NULL;

	delete backgroundMenu;
	backgroundMenu = NULL;
	audio_manager_->UnloadMusic();

	changingSFXVol = 0;
	changingMasterVol = 0;
	changingMusicVol = 0;
}

void SceneApp::FrontendUpdate(float frame_time)// welcome to "if" land!
{
	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);
	gef::Keyboard* keyboard = input_manager_->keyboard();// makes a local keyboard for the front end update so it can read keyboard input
	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_X)) {// if the player presses x, starts the game
		FrontendRelease();
		gameState = GAME;
		GameInit();
	}

	if (!changingMusicVol && !changingSFXVol && !changingMasterVol) {
		// changes difficulty and background colour (player cant see it as the background image is in the way)
		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_1)) {
			difficulty = 1;
			platform_.set_render_target_clear_colour(gef::Colour(0.5f, 0.8f, 0.5f));// rgba
		}
		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_2)) {
			difficulty = 2;
			platform_.set_render_target_clear_colour(gef::Colour(0.3f, 0.3f, 1.0f));// rgba
		}
		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_3)) {
			difficulty = 3;
			platform_.set_render_target_clear_colour(gef::Colour(0, 0, 0));// rgba
		}
	}

	// changes which volume is being edited and deselects all other flags
	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_M)) {
		changingMusicVol = !changingMusicVol;
		changingSFXVol = 0;
		changingMasterVol = 0;
	}

	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_S)) {
		changingSFXVol = !changingSFXVol;
		changingMusicVol = 0;
		changingMasterVol = 0;
	}

	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_A)) {
		changingMasterVol = !changingMasterVol;
		changingSFXVol = 0;
		changingMusicVol = 0;
	} 

#pragma region Sound Stuff
	// music volume
	if (changingMusicVol) {
		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_1)) {
			musicVolumeInfo.volume = 0.0f;
			musicVolText = 1;
			audio_manager_->SetMusicVolumeInfo(musicVolumeInfo);
		}
		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_2)) {
			musicVolumeInfo.volume = 10.0f;
			musicVolText = 2;
			audio_manager_->SetMusicVolumeInfo(musicVolumeInfo);
		}

		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_3)) {
			musicVolumeInfo.volume = 20.0f;
			musicVolText = 3;
			audio_manager_->SetMusicVolumeInfo(musicVolumeInfo);
		}

		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_4)) {
			musicVolumeInfo.volume = 40.0f;
			musicVolText = 4;
			audio_manager_->SetMusicVolumeInfo(musicVolumeInfo);
		}

		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_5)) {
			musicVolumeInfo.volume = 100.0f;
			musicVolText = 5;
			audio_manager_->SetMusicVolumeInfo(musicVolumeInfo);
		}
	}

	// SFX volume
	if (changingSFXVol) {
		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_1)) {
			SFXVolumeInfo.volume = 0.0f;
			SFXVolumeInfo.pan = 0.0f;
			SFXVolText = 1;


		}
		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_2)) {
			SFXVolumeInfo.volume = 10.0f;
			SFXVolText = 2;

		}

		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_3)) {
			SFXVolumeInfo.volume = 20.0f;
			SFXVolText = 3;

		}

		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_4)) {
			SFXVolumeInfo.volume = 40.0f;
			SFXVolText = 4;

		}

		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_5)) {
			SFXVolumeInfo.volume = 100.0f;
			SFXVolText = 5;

		}
	}

	// master volume
	if (changingMasterVol) {
		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_1)) {
			masterVolumeInfo.volume = 0.0f;
			masterVolText = 1;
			audio_manager_->SetMasterVolume(masterVolumeInfo.volume);
		}
		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_2)) {
			masterVolumeInfo.volume = 10.0f;
			masterVolText = 2;
			audio_manager_->SetMasterVolume(masterVolumeInfo.volume);
		}

		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_3)) {
			masterVolumeInfo.volume = 20.0f;
			masterVolText = 3;
			audio_manager_->SetMasterVolume(masterVolumeInfo.volume);
		}

		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_4)) {
			masterVolumeInfo.volume = 40.0f;
			masterVolText = 4;
			audio_manager_->SetMasterVolume(masterVolumeInfo.volume);
		}

		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_5)) {
			masterVolumeInfo.volume = 100.0f;
			masterVolText = 5;
			audio_manager_->SetMasterVolume(masterVolumeInfo.volume);
		}
	}
#pragma endregion
}// :)

void SceneApp::FrontendRender()
{
	sprite_renderer_->Begin();
	
	// render controls
	gef::Sprite background;
	background.set_texture(backgroundMenu);
	background.set_position(gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f, -0.99f));
	background.set_height(platform_.height());
	background.set_width(platform_.width());// 608 544
	sprite_renderer_->DrawSprite(background);


	// render clear heart or aced heart next to easy
	if (easyBeat) {
		if (easyAced) {
			gef::Sprite winGraphic;
			winGraphic.set_texture(aceHeart);
			winGraphic.set_position(gef::Vector4(platform_.width()*0.25f, platform_.height()*0.47f, -0.99f));// pos for easy text
			winGraphic.set_height(24.0f);
			winGraphic.set_width(27.0f);// 27 24
			sprite_renderer_->DrawSprite(winGraphic);
		}
		else {
			gef::Sprite winGraphic;
			winGraphic.set_texture(clearHeart);
			winGraphic.set_position(gef::Vector4(platform_.width()*0.25f, platform_.height()*0.47f, -0.99f));// pos for easy text
			winGraphic.set_height(24.0f);
			winGraphic.set_width(27.0f);// 27 24
			sprite_renderer_->DrawSprite(winGraphic);
		}
	}

	// render clear heart or aced heart next to regular
	if (regularBeat) {
		if (regularAced) {
			gef::Sprite winGraphic;
			winGraphic.set_texture(aceHeart);
			winGraphic.set_position(gef::Vector4(platform_.width()*0.25f, platform_.height()*0.54f, -0.99f));// pos for easy text
			winGraphic.set_height(24.0f);
			winGraphic.set_width(27.0f);// 27 24
			sprite_renderer_->DrawSprite(winGraphic);
		}
		else {
			gef::Sprite winGraphic;
			winGraphic.set_texture(clearHeart);
			winGraphic.set_position(gef::Vector4(platform_.width()*0.25f, platform_.height()*0.54f, -0.99f));// pos for easy text
			winGraphic.set_height(24.0f);
			winGraphic.set_width(27.0f);// 27 24
			sprite_renderer_->DrawSprite(winGraphic);
		}
	}

	// render clear heart or aced heart next to hard
	if (hardBeat) {
		if (hardAced) {
			gef::Sprite winGraphic;
			winGraphic.set_texture(aceHeart);
			winGraphic.set_position(gef::Vector4(platform_.width()*0.25f, platform_.height()*0.60f, -0.99f));// pos for easy text
			winGraphic.set_height(24.0f);
			winGraphic.set_width(27.0f);// 27 24
			sprite_renderer_->DrawSprite(winGraphic);
		}
		else {
			gef::Sprite winGraphic;
			winGraphic.set_texture(clearHeart);
			winGraphic.set_position(gef::Vector4(platform_.width()*0.25f, platform_.height()*0.60f, -0.99f));// pos for easy text
			winGraphic.set_height(24.0f);
			winGraphic.set_width(27.0f);// 27 24
			sprite_renderer_->DrawSprite(winGraphic);
		}
	}

	font_->RenderText(
		sprite_renderer_,
		gef::Vector4(platform_.width()*0.01f, platform_.height()*0.89f, -0.99f),
		1.0f,
		0xff000000,
		gef::TJ_LEFT,
		"Press 1, 2 or 3 to change the difficulty to: 1 = Easy --- 2 = Regular --- 3 = Hard");

	font_->RenderText(
		sprite_renderer_,
		gef::Vector4(platform_.width()*0.01f, platform_.height()*0.94f, -0.99f),
		1.0f,
		0xff000000,
		gef::TJ_LEFT,
		"Difficulty: %1.0f", float(difficulty));// apparently it wont display numbers unless it's a float, so mask time <(.-.)-4

	font_->RenderText(// renders text for music volume
		sprite_renderer_,
		gef::Vector4(platform_.width()*0.23f, platform_.height()*0.75f, -0.99f),
		1.0f,
		0xff000000,
		gef::TJ_LEFT,
		"%1.0f", float(musicVolText));

	font_->RenderText(// renders text for SFX volume
		sprite_renderer_,
		gef::Vector4(platform_.width()*0.23f, platform_.height()*0.80f, -0.99f),
		1.0f,
		0xff000000,
		gef::TJ_LEFT,
		"%1.0f", float(SFXVolText));

	font_->RenderText(// renders text for master volume
		sprite_renderer_,
		gef::Vector4(platform_.width()*0.23f, platform_.height()*0.85f, -0.99f),
		1.0f,
		0xff000000,
		gef::TJ_LEFT,
		"%1.0f", float(masterVolText));

	DrawHUD();
	sprite_renderer_->End();
}

void SceneApp::DeadInit() {
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	youDied = CreateTextureFromPNG("u ded.png", platform_);

	audio_manager_->PlaySample(playerDeadSound);// oOof.
}
void SceneApp::DeadRelease() {
	delete sprite_renderer_;
	sprite_renderer_ = NULL;
}
void SceneApp::DeadUpdate(float frame_time) {
	gef::Keyboard* keyboard = input_manager_->keyboard();// makes a local keyboard for the front end update so it can read keyboard input
	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_R)) {// if the player presses r (reset) then takes player back to main menu
		GameRelease();
		DeadRelease();
		gameState = LOAD;
		FrontendInit();
		return;
	}
}
void SceneApp::DeadRender() {
	gef::Sprite ded;
	ded.set_texture(youDied);
	ded.set_position(gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f, -0.99f));
	ded.set_height(205.0f);// dimentions of pic = 637 * 205
	ded.set_width(637.0f);
	sprite_renderer_->DrawSprite(ded);
}

void SceneApp::WinInit() {
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	youWin = CreateTextureFromPNG("u win.png", platform_);
	youAced = CreateTextureFromPNG("u ace.png", platform_);
	// determines if the player aced the game or not
	aced = 0;
	if (player->health >= 100) {
		switch (difficulty) {
		case 1: easyBeat = 1, easyAced = 1, aced = 1;
			break;

		case 2: regularBeat = 1, regularAced = 1, aced = 1;
			break;

		case 3: hardBeat = 1, hardAced = 1, aced = 1;
			break;

		}
	}
	else {
		switch (difficulty) {
		case 1: easyBeat = 1;
			break;

		case 2: regularBeat = 1;
			break;

		case 3: hardBeat = 1;
			break;

		}
	}
}
void SceneApp::WinRelease() {
	delete sprite_renderer_;
	sprite_renderer_ = NULL;

	delete youWin;
	youWin = NULL;

	delete youAced;
	youAced = NULL;

	aced = 0;
}
void SceneApp::WinUpdate(float frame_time) {
	gef::Keyboard* keyboard = input_manager_->keyboard();// makes a local keyboard for the front end update so it can read keyboard input
	//gef::Keyboard* keyboard = input_manager_->keyboard();// makes a local keyboard for the front end update so it can read keyboard input
	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_W)) {// if the player presses w (winner winner) then take them back to the main menu
		GameRelease();
		WinRelease();
		gameState = LOAD;
		FrontendInit();
		return;
	}
}
void SceneApp::WinRender() {

	if (aced) {// if the player aced the level, display u aced, if not, display u win
		gef::Sprite aced;

		aced.set_texture(youAced);
		aced.set_position(gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f, -0.99f));
		aced.set_height(205.0f);// dimentions of pic = 637 * 205
		aced.set_width(637.0f);
		sprite_renderer_->DrawSprite(aced);
	}
	else {
		gef::Sprite win;

		win.set_texture(youWin);
		win.set_position(gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f, -0.99f));
		win.set_height(205.0f);// dimentions of pic = 637 * 205
		win.set_width(637.0f);
		sprite_renderer_->DrawSprite(win);
	}




}

void SceneApp::PauseInit() {
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	youPaused = CreateTextureFromPNG("u paused.png", platform_);
}
void SceneApp::PauseRelease() {
	delete sprite_renderer_;
	sprite_renderer_ = NULL;
}
void SceneApp::PauseUpdate(float frame_time) {
	gef::Keyboard* keyboard = input_manager_->keyboard();// makes a local keyboard for the front end update so it can read keyboard input
	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_F)) {// f takes the player back to the main menu
		GameRelease();
		PauseRelease();
		gameState = LOAD;

		FrontendInit();
		return;
	}

	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_P)) {// p un-pauses the game
		gameState = GAME;
		return;
	}
}
void SceneApp::PauseRender() {
	gef::Sprite pause;
	pause.set_texture(youPaused);
	pause.set_position(gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f, -0.99f));
	pause.set_height(205.0f);// dimentions of pic = 637 * 205
	pause.set_width(637.0f);
	sprite_renderer_->DrawSprite(pause);
}


void SceneApp::GameInit()
{
	// loads all the sound effects for the game
	hitmarkerSound = audio_manager_->LoadSample("hitmarker.wav", platform_);
	winSound = audio_manager_->LoadSample("winner winner.wav", platform_);
	audio_manager_->LoadMusic("gatman game music.wav", platform_);
	playerDeadSound = audio_manager_->LoadSample("oof.wav", platform_);
	enemyShotSound = audio_manager_->LoadSample("enemy shot.wav", platform_);
	gatmanShotSound = audio_manager_->LoadSample("gatmanGat.wav", platform_);

	audio_manager_->PlayMusic();// starts the game's banging beats

	// sets up free cam at default position
	freeCam = new Camera();
	playerCam = new Camera();
	whatCam = 1;

	freeCam->setPosition(gef::Vector4(-5.0f, -20.0f, 5.0f));
	freeCam->setLookAt(gef::Vector4(0.0f, 0.0f, 0.0f));

	// create the renderer for draw 3D geometry
	renderer_3d_ = gef::Renderer3D::Create(platform_);

	// initialise primitive builder to make create some 3D geometry easier
	primitive_builder_ = new PrimitiveBuilder(platform_);



	SetupLights();

	// initialise the physics world
	b2Vec2 gravity(0.0f, -9.81f);
	world = new b2World(gravity);
	enemyCount = 0;

	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	//audio_manager_ = gef::AudioManager::Create();
	//soundBoxCollected = audio_manager_->LoadSample("box_collected.wav", platform_);
	//audio_manager_->LoadMusic("music.wav", platform_);
	switch (difficulty) {// depending on the diffiuclty seleced, changes platform cound and the enemy health modifyer accordingly
	case 1: {
		platformCount = 15;// was 10 and probably should be
		enemyHealthMod = 20;

	}
			break;

	case 2: {
		platformCount = 30;// was 15 and probably should be
		enemyHealthMod = 50;

	}
			break;

	case 3: {
		platformCount = 50;// was 20 and probably should be
		enemyHealthMod = 100;


	}
			break;
	}
	for (int i = 0; i < platformCount; i++)// initilizes the enemy randomized pattern cast
		enemiesCast.push_back(0);

	InitPlayer();
	InitGround();
	InitEnemies();
	InitGoal();

	deadFlag = 0;
	winFlag = 0;
	// if this isn't here, all the sfx takes a stronk
	audio_manager_->SetSampleVoiceVolumeInfo(enemyShotSound, SFXVolumeInfo);
	audio_manager_->SetSampleVoiceVolumeInfo(hitmarkerSound, SFXVolumeInfo);
	audio_manager_->SetSampleVoiceVolumeInfo(playerDeadSound, SFXVolumeInfo);
	audio_manager_->SetSampleVoiceVolumeInfo(winSound, SFXVolumeInfo);
	audio_manager_->SetSampleVoiceVolumeInfo(gatmanShotSound, SFXVolumeInfo);
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

	delete scene_assets_;
	scene_assets_ = NULL;

	delete sprite_renderer_;
	sprite_renderer_ = NULL;

	delete freeCam;
	freeCam = NULL;

	delete playerCam;
	playerCam = NULL;

	/*delete player;// might not be needed
	player = NULL;

	delete ground;
	ground = NULL;

	for (int i = 0; i < enemies.size(); i++) {
		delete enemies[i];
		enemies[i] = NULL;
	}

	for (int i = 0; i < platforms.size(); i++) {
		delete platforms[i];
		platforms[i] = NULL;
	}*/


	for (int i = 0; i < enemiesCast.size(); i++) {// resets the enemy pattern cast
		enemiesCast[i] = 0;
	}

	for (int i = 0; i < enemies.size(); i++) {
		delete enemies[i];
		enemies[i] = NULL;
	}
	enemies.clear();// if these aren't here everything takes a stronk, clears vector compleatly

	for (int i = 0; i < platforms.size(); i++) {
		delete platforms[i];
		platforms[i] = NULL;
	}
	platforms.clear();// if these aren't here everything takes a stronk, clears vector compleatly

	audio_manager_->UnloadMusic();

	deadFlag = 0;
	winFlag = 0;
}

void SceneApp::GameUpdate(float frame_time)
{
	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);

	UpdateSimulation(frame_time);// crashes here 

	gef::Keyboard* keyboard = input_manager_->keyboard();// makes a local keyboard for the front end update so it can read keyboard input

	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_P)) {
		PauseInit();
		gameState = PAUSE;

		return;
	}

#pragma region Switch Cams
	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_0)) {// switches to freecam
		whatCam = 0;
	}

	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_1)) {// switches to playercam
		whatCam = 1;
	}

	if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_2)) {// switches to freezecam
		whatCam = 2;
	}
#pragma endregion

#pragma region Player Controls
	if (whatCam != 0) {
		if (!player->isDead) {// only lets the user control the player if the camera isnt on the free cam
			player->playerUpdateControls(frame_time, keyboard);
			player->playerUpdate(frame_time);
		}


	}

#pragma endregion

#pragma region Freecam Controls
	if (whatCam == 0) {
		// move controls
		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_W)) {// makes freecam move forward
			freeCam->moveForward(frame_time);
		}
		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_S)) {// makes freecam move right
			freeCam->moveBack(frame_time);
		}
		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_A)) {// makes freecam move right
			freeCam->moveLeft(frame_time);
		}
		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_D)) {// makes freecam move right
			freeCam->moveRight(frame_time);
		}

		// rot controls
		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_UP)) {// makes freecam rotate up
			freeCam->rotUp(frame_time);
		}

		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_DOWN)) {// makes freecam rotate down
			freeCam->rotDown(frame_time);
		}

		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_LEFT)) {// makes freecam rotate left
			freeCam->rotLeft(frame_time);
		}

		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_RIGHT)) {// makes freecam rotate right
			freeCam->rotRight(frame_time);
		}
	}


	playerCam->setPosition(gef::Vector4(player->playerBody->GetPosition().x + 3.5f, player->playerBody->GetPosition().y + 1.0f, 10.0f));// <-- this does infact work, sets the position of the camea to be a little bit off set for the center of the player so you can see that he has a hat on
	playerCam->setLookAt(gef::Vector4(player->playerBody->GetPosition().x, player->playerBody->GetPosition().y, 0.0f));// its just the camera is still only looking up rather than at the box :/

#pragma endregion

	b2Vec2 tempVelo = player->playerBody->GetLinearVelocity();
	tempVelo.x *= 0.95;// slows the box's velocity in the x axis down over time so its not slipping off the platforms
	player->playerBody->SetLinearVelocity(tempVelo);

	// updates cameras
	playerCam->update();
	playerCam->updateLookAt();

	freeCam->update();
	freeCam->updateLookAt();

	// updates goal
	goal->goalUpdate(frame_time);

	// updates enemys with dt and player's position
	b2Vec2 playerPos = player->playerBody->GetPosition();// this is done first to reduce the amount of accessses to playerbody

	for (int i = 0; i < enemies.size(); i++) {
		enemies[i]->enemyUpdate(frame_time, playerPos);
	}

	// updates player's bullets
	for (int i = 0; i < player->bullets.size(); i++) {
		if (player->bullets[i] == NULL) {

		}
		else
			player->bullets[i]->bulletUpdate(frame_time);
	}

	// updates enemy's bullets
	for (int i = 0; i < enemies.size(); i++) {
		for (int j = 0; j < enemies[i]->bullets.size(); j++) {
			if (enemies[i]->bullets[j] == NULL) {

			}
			else {
				enemies[i]->bullets[j]->bulletUpdate(frame_time);// crashes here :( bullet isnt initilised due to the argument being null (in vertex buffer) REEEEEEEE
			}
		}
	}
	if (!deadFlag) {
		if (player->isDead) {
			gameState = DEAD;
			// play death sound and music (pls use roblox death ones) in dead init <<--- this was the first comment in this section and I refuse to take it out
			DeadInit();
			deadFlag = 1;
		}
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
	case 0: {	view_matrix.LookAt(freeCam->getPosition(), freeCam->getLookAt(), freeCam->getUp());
		renderer_3d_->set_view_matrix(view_matrix); };
		break;

	case 1: {	view_matrix.LookAt(playerCam->getPosition(), playerCam->getLookAt(), playerCam->getUp());
		renderer_3d_->set_view_matrix(view_matrix); };
		break;

	}


	// draw 3d geometry
	renderer_3d_->Begin();

		// draw ground
		renderer_3d_->DrawMesh(*ground);
		for (int i = 0; i < platforms.size(); i++) {
			renderer_3d_->DrawMesh(*platforms[i]);
		}
		renderer_3d_->set_override_material(&primitive_builder_->red_material());
		for (int i = 0; i < enemies.size(); i++) {

			renderer_3d_->DrawMesh(*enemies[i]);
		}
		renderer_3d_->set_override_material(NULL);

		// renders player's bullets
		for (int i = 0; i < player->bullets.size(); i++) {
			if (player->bullets[i] == NULL) {
			
			}
			else {
				renderer_3d_->DrawMesh(*player->bullets[i]);
			}
		}

		// renders enemy's bullets
		for (int i = 0; i < enemies.size(); i++) {
			for(int j = 0; j < enemies[i]->bullets.size(); j++)
			if (enemies[i]->bullets[j] == NULL) {

			}
			else {
				renderer_3d_->DrawMesh(*enemies[i]->bullets[j]);
			}
		}



		// renders player
		renderer_3d_->set_override_material(&primitive_builder_->blue_material());
		renderer_3d_->DrawMesh(*player);
		renderer_3d_->set_override_material(NULL);

		// renders goal
		renderer_3d_->set_override_material(&primitive_builder_->green_material());
		renderer_3d_->DrawMesh(*goal);
		renderer_3d_->set_override_material(NULL);

	renderer_3d_->End();

	// start drawing sprites, but don't clear the frame buffer
	sprite_renderer_->Begin(false);
	DrawHUD();
	sprite_renderer_->End();
}



