#ifndef _SCENE_APP_H
#define _SCENE_APP_H

#include <system/application.h>
#include <maths/vector2.h>
#include "primitive_builder.h"
#include <graphics/mesh_instance.h>
#include <input/input_manager.h>
#include <audio/audio_manager.h>
#include <box2d/Box2D.h>
#include "game_object.h"
#include "Camera.h"
#include <vector>
#include <random>
#include <time.h>

// FRAMEWORK FORWARD DECLARATIONS
enum GAMESTATE {
	INTRO,
	LOAD,
	PAUSE,
	GAME,
	DEAD,
	WIN
};


namespace gef// init for geffery
{
	class Platform;
	class SpriteRenderer;
	class Font;
	class InputManager;
	class Renderer3D;
	class Scene;
	class Audio;
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
	void InitGoal();
	void CleanUpFont();
	void DrawHUD();
	void SetupLights();
	void UpdateSimulation(float frame_time);
    

	gef::SpriteRenderer* sprite_renderer_;
	gef::Font* font_;
	gef::InputManager* input_manager_;
	gef::AudioManager* audio_manager_;
	// progress info declerations
	int difficulty;
	bool easyBeat;
	bool easyAced;
	bool regularBeat;
	bool regularAced;
	bool hardBeat;
	bool hardAced;

	b2Timer timer;// timer for the intro

	// audio variables
	int32 introSound = -1;// loading the audio sample returns some info we store here so we can call it 
	int32 gatmanShotSound = -1;
	int32 enemyShotSound = -1;
	int32 hitmarkerSound = -1;
	int32 playerDeadSound = -1;
	int32 winSound = -1;
	int32 secretSound = -1;

	//int32 introVID = -1;// calling the sound returns this voice id (VID) so we can do stuff like stop it, make it louder, etc
	gef::VolumeInfo musicVolumeInfo;// controls the SFX's volume
	gef::VolumeInfo SFXVolumeInfo;// controls the SFX's volume
	gef::VolumeInfo masterVolumeInfo;// controls the SFX's volume
	int musicVolText, SFXVolText, masterVolText;
	//
	// INTRO DECLARATIONS
	//
	std::vector<std::string> quotes;
	int whatQuote;// represents what quote it's gonna be
	gef::Texture* background;// for some reason textures must be pointers
	//
	// FRONTEND DECLARATIONS
	//
	gef::Texture* button_icon_;
	gef::Texture* backgroundMenu;
	gef::Texture* clearHeart;
	gef::Texture* aceHeart;

	bool changingMusicVol, changingSFXVol, changingMasterVol;
	//
	// GAME DECLARATIONS
	//
	gef::Texture* easyBackground;
	gef::Texture* regularBackground;
	gef::Texture* hardBackground;

	gef::Renderer3D* renderer_3d_;
	PrimitiveBuilder* primitive_builder_;
	gef::Scene* scene_assets_;
	GAMESTATE gameState;
	int platformCount;
	int enemyHealthMod;// modifier to add extra health to enemys

	//
	// DEAD DECLARATIONS
	//
	gef::Texture* youDied;
	bool deadFlag;

	//
	// WIN DECLARATIONS
	//
	gef::Texture* youWin;
	gef::Texture* youAced;
	bool aced;
	bool winFlag;

	//
	// PAUSE DECLARATIONS
	//
	gef::Texture* youPaused;

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



	// loading the music doesn't return info
	bool isMusicPlaying = 0;// toggle for music is playing
	bool isMusicEarrape = 0;

	Goal* goal;


	// cameras
	int whatCam;// variable that keeps track of what cam is being used: 0 = freecam, 1 = playercam
	Camera* freeCam;
	Camera* playerCam;

	float fps_;

	void IntroInit();
	void IntroRelease();
	void IntroUpdate(float frame_time);
	void IntroRender();

	void FrontendInit();
	void FrontendRelease();
	void FrontendUpdate(float frame_time);
	void FrontendRender();

	void PauseInit();
	void PauseRelease();
	void PauseUpdate(float frame_time);
	void PauseRender();

	void GameInit();
	void GameRelease();
	void GameUpdate(float frame_time);
	void GameRender();

	void DeadInit();
	void DeadRelease();
	void DeadUpdate(float frame_time);
	void DeadRender();

	void WinInit();
	void WinRelease();
	void WinUpdate(float frame_time);
	void WinRender();

	//void playerUpdate(float frame_time);
};

#endif // _SCENE_APP_H
