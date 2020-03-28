#pragma once
#include <system/application.h>
#include <maths/vector2.h>
#include "primitive_builder.h"
#include <graphics/mesh_instance.h>
#include <input/input_manager.h>
#include <box2d/Box2D.h>
class Camera
{
	/*	gef::Vector4 camera_eye(-2.0f, 2.0f, 10.0f);
	gef::Vector4 camera_lookat(0.0f, 0.0f, 0.0f);
	gef::Vector4 camera_up(0.0f, 1.0f, 0.0f);
	gef::Matrix44 view_matrix;
	view_matrix.LookAt(camera_eye, camera_lookat, camera_up);
	renderer_3d_->set_view_matrix(view_matrix);*/

public:
	Camera();
	void update();
	void updateLookAt();// updates the look at vector, this is neded as the ship class edits the lookat vector

	/**getters n' setters*/
	gef::Vector4 getPosition();
	void setPosition(gef::Vector4 setPosition);

	gef::Vector4 getLookAt();
	void setLookAt(gef::Vector4 setLookAt);

	gef::Vector4 getUp();
	void setUp(gef::Vector4 setUp);

	gef::Vector4 getRotation();
	void setRotation(gef::Vector4 setRotation);

	gef::Vector4 getRight();
	void setRight(gef::Vector4 setRight);

	float getSpeed();
	void setSpeed(float setSpeed);

	float getSensitivity();
	void setSensitivity(float setSensitivity);

	/**end of getters n' setters*/

	/**movement functions*/
	void moveForward(float dt);
	void moveBack(float dt);
	void moveRight(float dt);
	void moveLeft(float dt);

	void rotUp(float dt);
	void rotDown(float dt);
	void rotRight(float dt);
	void rotLeft(float dt);
	void rollRight(float dt);
	void rollLeft(float dt);
	/**end of movement functions*/
private:
	//vectors
	gef::Vector4 position;
	gef::Vector4 lookAt;
	gef::Vector4 up;
	gef::Vector4 rotation;// x is pitch, y is yaw and z is roll
	gef::Vector4 right;
	gef::Vector4 forward; // the vector representing forward

	float cosR, cosP, cosY;// temp values for sin/cos from float sinR, sinP, sinY
	float sinR, sinP, sinY;

	float speed;
	float sensitivity;

	float centreX;
	float centreY;
};

