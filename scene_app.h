#ifndef _SCENE_APP_H
#define _SCENE_APP_H

#include <system/application.h>
#include <maths/vector2.h>
#include "primitive_builder.h"
#include <graphics/mesh_instance.h>
#include <input/input_manager.h>
#include <box2d/Box2D.h>
#include "game_object.h"
#include "Camera.h"
#include <vector>
#include <random>


// FRAMEWORK FORWARD DECLARATIONS
enum GAMESTATE {
	LOAD,
	GAME
};


namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Font;
	class InputManager;
	class Renderer3D;
}

class SceneApp : public gef::Application
{
public:
	SceneApp(gef::Platform& platform);
	void Init();
	void CleanUp();
	bool Update(float frame_time);
	void Render();
private:
	void InitPlayer();
	void InitGround();
	void InitFont();
	void CleanUpFont();
	void DrawHUD();
	void SetupLights();
	void UpdateSimulation(float frame_time);
    
	gef::SpriteRenderer* sprite_renderer_;
	gef::Font* font_;
	gef::InputManager* input_manager_;


	//
	// FRONTEND DECLARATIONS
	//
	gef::Texture* button_icon_;

	//
	// GAME DECLARATIONS
	//
	gef::Renderer3D* renderer_3d_;
	PrimitiveBuilder* primitive_builder_;
	GAMESTATE gameState;

	// create the physics world
	b2World* world;

	// player variables
	Player player;


	// ground variables
	Floor ground;
	std::vector<Floor*> platforms;

	// audio variables
	int sfx_id_;
	int sfx_voice_id_;

	// cameras
	int whatCam;// variable that keeps track of what cam is being used: 0 = freecam, 1 = playercam
	Camera freeCam;
	Camera playerCam;

	float fps_;

	void FrontendInit();
	void FrontendRelease();
	void FrontendUpdate(float frame_time);
	void FrontendRender();

	void GameInit();
	void GameRelease();
	void GameUpdate(float frame_time);
	void GameRender();

	void playerUpdate(float frame_time);
};

#endif // _SCENE_APP_H
