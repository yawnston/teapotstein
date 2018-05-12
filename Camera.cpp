#define _USE_MATH_DEFINES

#include <iostream>
#include <cmath>
#include <GL/glut.h>
#include "Camera.h"

GLfloat light_p[] = { 5.0f, 0.0f, 5.0f, 0.0f };

void Camera::init()
{
	yaw = 0.0;
	pitch = 0.0;
	set_pos(0, 0, 0);
}

void Camera::refresh()
{
	// x/y for flat, z for height
	dir_x = cos(yaw) * cos(pitch);
	dir_y = sin(pitch);
	dir_z = sin(yaw) * cos(pitch);

	dir_strafe_x = (float)cos(yaw - M_PI_2);
	dir_strafe_z = (float)sin(yaw - M_PI_2);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(pos_x, pos_y, pos_z, // Look from camera XYZ
		pos_x + dir_x, pos_y + dir_y, pos_z + dir_z, // Look at this spot
		0.0, 1.0, 0.0); // Positive Y up vector

	glLightfv(GL_LIGHT0, GL_POSITION, light_p);
	glLoadIdentity();
	gluLookAt(pos_x, pos_y, pos_z, // Look from camera XYZ
		pos_x + dir_x, pos_y + dir_y, pos_z + dir_z, // Look at this spot
		0.0, 1.0, 0.0); // Positive Y up vector
}

void Camera::set_pos(float x, float y, float z)
{
	pos_x = x;
	pos_y = y;
	pos_z = z;
	refresh();
}

void Camera::get_pos(float& x, float& y, float& z)
{
	x = pos_x;
	y = pos_y;
	z = pos_z;
}

void Camera::get_direction(float& x, float& y, float& z)
{
	x = dir_x;
	y = dir_y;
	z = dir_z;
}

void Camera::move_forward(float incr)
{
	float movement_x = cos(yaw)*cos(pitch);
	float movement_z = sin(yaw)*cos(pitch);

	// uncomment this to enable flying with WASD (enemies don't know how to take this into account! use with caution)
	//float movement_y = sin(m_pitch);
	//pos_y = pos_y + incr*movement_y;

	pos_x = pos_x + incr*movement_x;
	pos_z = pos_z + incr*movement_z;
	refresh();
}

void Camera::move_sideways(float incr)
{
	pos_x = pos_x + incr*dir_strafe_x;
	pos_z = pos_z + incr*dir_strafe_z;
	refresh();
}

void Camera::move_upwards(float incr)
{
	pos_y = pos_y + incr;
	refresh();
}

void Camera::adjust_yaw(float angle)
{
	yaw += angle;
	refresh();
}

void Camera::adjust_pitch(float angle)
{
	// we can't look directly up
	const float limit = (float)(89.0 * M_PI / 180.0);
	pitch -= angle;
	if (pitch < -limit) pitch = -limit;
	if (pitch > limit) pitch = limit;
	refresh();
}

void Camera::set_yaw(float angle)
{
	yaw = angle;
	refresh();
}

void Camera::set_pitch(float angle)
{
	pitch = angle;
	refresh();
}
