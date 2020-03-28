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
	world_(NULL),
	player_body_(NULL),
	button_icon_(NULL)
{
}

void SceneApp::Init()
{
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	InitFont();

	// initialise input manager
	input_manager_ = gef::InputManager::Create(platform_);

	gameState = LOAD;
	FrontendInit();

}

void SceneApp::CleanUp()
{
	delete input_manager_;
	input_manager_ = NULL;

	CleanUpFont();

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
	player_.set_mesh(primitive_builder_->GetDefaultCubeMesh());

	// create a physics body for the player
	b2BodyDef player_body_def;
	player_body_def.type = b2_dynamicBody;
	player_body_def.position = b2Vec2(0.0f, 4.0f);

	player_body_ = world_->CreateBody(&player_body_def);

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
	player_.UpdateFromSimulation(player_body_);

	// create a connection between the rigid body and GameObject
	player_body_->SetUserData(&player_);

	freeCam.setPosition(gef::Vector4(player_body_->GetPosition().x, player_body_->GetPosition().y, 0.0f));
	//freeCam.setLookAt(gef::Vector4(player_body_->GetPosition().x, player_body_->GetPosition().y, 0.0f));

	playerCam.setPosition(gef::Vector4(player_body_->GetPosition().x, player_body_->GetPosition().y, 0.0f));
	//playerCam.setLookAt(gef::Vector4(player_body_->GetPosition().x, player_body_->GetPosition().y, 0.0f));
}

void SceneApp::InitGround()
{
	// ground dimensions
	gef::Vector4 ground_half_dimensions(5.0f, 0.5f, 0.5f);

	// setup the mesh for the ground
	ground_mesh_ = primitive_builder_->CreateBoxMesh(ground_half_dimensions);
	ground_.set_mesh(ground_mesh_);

	// create a physics body
	b2BodyDef body_def;
	body_def.type = b2_staticBody;
	body_def.position = b2Vec2(0.0f, 0.0f);

	ground_body_ = world_->CreateBody(&body_def);

	// create the shape
	b2PolygonShape shape;
	shape.SetAsBox(ground_half_dimensions.x(), ground_half_dimensions.y());

	// create the fixture
	b2FixtureDef fixture_def;
	fixture_def.shape = &shape;

	// create the fixture on the rigid body
	ground_body_->CreateFixture(&fixture_def);

	// update visuals from simulation data
	ground_.UpdateFromSimulation(ground_body_);
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

	world_->Step(timeStep, velocityIterations, positionIterations);

	// update object visuals from simulation data
	player_.UpdateFromSimulation(player_body_);
	player_.playerUpdate(frame_time);
	// don't have to update the ground visuals as it is static

	// collision detection
	// get the head of the contact list
	b2Contact* contact = world_->GetContactList();
	// get contact count
	int contact_count = world_->GetContactCount();

	for (int contact_num = 0; contact_num<contact_count; ++contact_num)
	{
		if (contact->IsTouching())
		{
			// get the colliding bodies
			b2Body* bodyA = contact->GetFixtureA()->GetBody();
			b2Body* bodyB = contact->GetFixtureB()->GetBody();

			// DO COLLISION RESPONSE HERE
			Player* player = NULL;

			GameObject* gameObjectA = NULL;
			GameObject* gameObjectB = NULL;

			gameObjectA = (GameObject*)bodyA->GetUserData();
			gameObjectB = (GameObject*)bodyB->GetUserData();

			if (gameObjectA)
			{
				if (gameObjectA->type() == PLAYER)
				{
					player = (Player*)bodyA->GetUserData();
				}
			}

			if (gameObjectB)
			{
				if (gameObjectB->type() == PLAYER)
				{
					player = (Player*)bodyB->GetUserData();
				}
			}

			if (player)
			{
				player->DecrementHealth();
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
	// sets up cameras at default positions
	whatCam = 0;
	//gef::Vector4 startCameraEye(-5.0f, -10.0f, 0.0f);
	//gef::Vector4 startCameraLookat(0.0f, 0.0f, 0.0f);
	//gef::Vector4 startCameraUp(0.0f, 1.0f, 0.0f);

	//freeCam.setPosition(startCameraEye);
	//freeCam.setLookAt(startCameraLookat);
	//freeCam.setUp(startCameraUp);

	//playerCam.setPosition(startCameraEye);
	//playerCam.setLookAt(gef::Vector4(0.0f,0.0f,0.0f));
	//playerCam.setUp(startCameraUp);
	// create the renderer for draw 3D geometry
	renderer_3d_ = gef::Renderer3D::Create(platform_);

	// initialise primitive builder to make create some 3D geometry easier
	primitive_builder_ = new PrimitiveBuilder(platform_);


	SetupLights();

	// initialise the physics world
	b2Vec2 gravity(0.0f, -9.81f);
	world_ = new b2World(gravity);

	InitPlayer();
	InitGround();
}

void SceneApp::GameRelease()
{
	// destroying the physics world also destroys all the objects within it
	delete world_;
	world_ = NULL;

	delete ground_mesh_;
	ground_mesh_ = NULL;

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
		if (input_manager_->keyboard() && keyboard->IsKeyPressed(gef::Keyboard::KC_SPACE)) {// makes player jump

			player_body_->ApplyForceToCenter(player_.jumpVelocity, 1);
		}

		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_RIGHT)) {// makes player move right
			player_body_->SetLinearVelocity(b2Vec2(player_.moveVelocity.x, player_body_->GetLinearVelocity().y));
		}

		if (input_manager_->keyboard() && keyboard->IsKeyDown(gef::Keyboard::KC_LEFT)) {// makes player move right
			player_body_->SetLinearVelocity(b2Vec2(-player_.moveVelocity.x, player_body_->GetLinearVelocity().y));
		}


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

	playerCam.setPosition(gef::Vector4(player_body_->GetPosition().x, player_body_->GetPosition().y - 10.0f, 0.0f));// <-- this does infact work, 
	playerCam.setLookAt(gef::Vector4(player_body_->GetPosition().x, player_body_->GetPosition().y, 0.0f));// its just the camera is still only looking up rather than at the box :/
	
#pragma endregion

	


	b2Vec2 tempVelo = player_body_->GetLinearVelocity();
	tempVelo.x *= 0.95;// slows the box's velocity in the x axis down over time so its not slipping off the platforms
	player_body_->SetLinearVelocity(tempVelo);

	/*if (controller->buttons_down() & gef_SONY_CTRL_CIRCLE) {
		GameRelease();
		gameState = LOAD;
		FrontendInit();
	}*/
	playerCam.updateLookAt();
	playerCam.update();

	freeCam.updateLookAt();
	freeCam.update();


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
	renderer_3d_->DrawMesh(ground_);

	// draw player
	renderer_3d_->set_override_material(&primitive_builder_->red_material());
	renderer_3d_->DrawMesh(player_);
	renderer_3d_->set_override_material(NULL);

	renderer_3d_->End();

	// start drawing sprites, but don't clear the frame buffer
	sprite_renderer_->Begin(false);
	DrawHUD();
	sprite_renderer_->End();
}
// controls the cameras rotation with the mouse



