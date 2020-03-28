#include "Camera.h"
Camera::Camera() {// constructor
	position = { 0.0f, 0.0f, 6.0f, 0.0f };
	lookAt = { 0.0f, 0.0f, -1.0f, 0.0f };
	up = { 0.0f, 1.0f, 0.0f, 0.0f };
	forward = { 0.0f, 0.0f , 0.0f, 0.0f };
	right = { 0.0f, 0.0f , 0.0f, 0.0f };
	speed = 1000;
	sensitivity = 300;
}


void Camera::update() {
	// calculates the horizontal and vertical components for yaw, pitch and roll
	cosY = cosf(rotation.y() * 3.1415 / 180);
	cosP = cosf(rotation.x() * 3.1415 / 180);
	cosR = cosf(rotation.z() * 3.1415 / 180);
	sinY = sinf(rotation.y() * 3.1415 / 180);
	sinP = sinf(rotation.x() * 3.1415 / 180);
	sinR = sinf(rotation.z() * 3.1415 / 180);

	// calculates forward 
	forward.set_x(sinY * cosP);
	forward.set_y(sinP);
	forward.set_z(cosP * -cosY);


	// calculates up with the rotation
	up.set_x(-cosY * sinR - sinY * sinP * cosR);
	up.set_y(cosP * cosR);
	up.set_z(-sinY * sinR - sinP * cosR * -cosY);
}

void Camera::updateLookAt() {
	lookAt = forward + position;
}

/**getters n' setters*/
gef::Vector4 Camera::getPosition() {
	return position;
}
void Camera::setPosition(gef::Vector4 setPosition) {
	position = setPosition;
}

gef::Vector4 Camera::getLookAt() {
	return lookAt;
}
void Camera::setLookAt(gef::Vector4 setLookAt) {
	lookAt = setLookAt;
}

gef::Vector4 Camera::getUp() {
	return up;
}
void Camera::setUp(gef::Vector4 setUp) {
	up = setUp;
}

gef::Vector4 Camera::getRotation() {
	return rotation;
}
void Camera::setRotation(gef::Vector4 setRotation) {
	rotation = setRotation;
}

gef::Vector4 Camera::getRight() {
	return right;
}
void Camera::setRight(gef::Vector4 setRight) {
	right = setRight;
}

float Camera::getSpeed() {
	return speed;
}
void Camera::setSpeed(float setSpeed) {
	speed = setSpeed;
}

float Camera::getSensitivity() {
	return sensitivity;
}
void Camera::setSensitivity(float setSensitivity) {
	sensitivity = setSensitivity;
}
/**end of getters n' setters*/

/**movement functions*/

// moves forward, back left and right
void Camera::moveForward(float dt) {
	// position += forward + speed * dt;// this is what it was
	forward.operator*(dt * speed);
	position.operator+= (forward);

}

void Camera::moveBack(float dt) {
	forward.operator*(dt * speed);
	position.operator-= (forward);
}

void Camera::moveRight(float dt) {
	// calculates right with the cross product of forward and up
	right = { forward.y() * up.z() - forward.z() * up.y(),
		forward.z() * up.x() - forward.x() * up.z(),
		forward.x()* up.y()- forward.y()* up.x()};
	right.operator*(dt * speed);
	position.operator+= (right);
}

void Camera::moveLeft(float dt) {
	// calculates right with the cross product of forward and up
	right = { forward.y()* up.z()-forward.z()*up.y(),
		forward.z()*up.x()-forward.x()* up.z(),
		forward.x()* up.y()- forward.y()* up.x()};
	right.operator*(dt * speed);
	position.operator-= (right);
}

// rotates the camera with the sensitivity variable
void Camera::rotUp(float dt) {
	rotation.set_x(rotation.x() + (sensitivity * dt));// these will probably need fixed later, i think it should be * rather than += but we will find out later lmao
}

void Camera::rotDown(float dt) {
	rotation.set_x(rotation.x() - sensitivity * dt);
}

void Camera::rotRight(float dt) {
	rotation.set_y(rotation.y() + sensitivity * dt);
}

void Camera::rotLeft(float dt) {
	rotation.set_y(rotation.y() - sensitivity * dt);
}

void Camera::rollRight(float dt) {
	rotation.set_z(rotation.z() - sensitivity * dt);// since the sensitivity works un-evenly, dt is doubled to make it work a bit better
}

void Camera::rollLeft(float dt) {
	rotation.set_z(rotation.z() + sensitivity * dt);
}

/**end of movement functions*/