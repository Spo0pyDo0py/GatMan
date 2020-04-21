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
#include <time.h>

#define PLATFORM_COUNT 30

// FRAMEWORK FORWARD DECLARATIONS
enum GAMESTATE {
	INTRO,
	LOAD,
	PAUSE,
	GAME,
	DEAD
};


namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Font;
	class InputManager;
	class Renderer3D;
	class Scene;
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
	void InitEnemies();
	void InitFont();
	void CleanUpFont();
	void DrawHUD();
	void SetupLights();
	void UpdateSimulation(float frame_time);
    
	gef::SpriteRenderer* sprite_renderer_;
	gef::Font* font_;
	gef::InputManager* input_manager_;
	//gef::AudioManager* audio_manager_;


	//
	// FRONTEND DECLARATIONS
	//
	gef::Texture* button_icon_;

	//
	// GAME DECLARATIONS
	//
	gef::Renderer3D* renderer_3d_;
	PrimitiveBuilder* primitive_builder_;
	gef::Scene* scene_assets_;
	GAMESTATE gameState;

	// create the physics world
	b2World* world;

	// player variables
	Player* player;


	// ground variables
	Floor* ground;
	std::vector<Floor*> platforms;

	// enemy variables
	std::vector<Enemy*> enemies;
	std::vector<bool> enemiesCast;
	int enemyCount;

	// audio variables
//	int32 soundBoxCollected = -1;// loading the audio sample returns some info we store here so we can call it 
//	int32 soundBoxCollectedVID = -1;// calling the sound returns this voice id (VID) so we can do stuff like stop it, make it louder, etc
//	gef::VolumeInfo soundVolumeInfo;// controlls the sound's volume
	//bool isSoundLoud = 0;// toggle for volume controlled by dpad

	// loading the music doesn't return info
	bool isMusicPlaying = 0;// toggle for music is playing
	bool isMusicEarrape = 0;

	// cameras
	int whatCam;// variable that keeps track of what cam is being used: 0 = freecam, 1 = playercam
	Camera freeCam;
	Camera playerCam;

	float fps_;

	/*void IntroInit();
	void IntroRelese();
	void IntroUpdate(float frame_time);
	void IntroRender();*/

	void FrontendInit();
	void FrontendRelease();
	void FrontendUpdate(float frame_time);
	void FrontendRender();

	/*void PauseInit();
	void PauseRelease();
	void PauseUpdate(float frame_time);
	void PauseRender();*/

	void GameInit();
	void GameRelease();
	void GameUpdate(float frame_time);
	void GameRender();

	void DeadInit();
	void DeadRelese();
	void DeadUpdate(float frame_time);
	void DeadRender();

	//void playerUpdate(float frame_time);
};

#endif // _SCENE_APP_H
