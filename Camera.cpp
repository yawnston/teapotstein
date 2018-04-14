#define _USE_MATH_DEFINES

#include <iostream>
#include <cmath>
#include <GL/glut.h>
#include "Camera.h"

GLfloat light_p[] = {5.0f, 0.0f, 5.0f, 0.0f};

void Camera::init()
{
	m_yaw = 0.0;
	m_pitch = 0.0;

	set_pos(0, 0, 0);
}

void Camera::refresh()
{
	// x/y for flat, z for height
	m_lx = cos(m_yaw) * cos(m_pitch);
	m_ly = sin(m_pitch);
	m_lz = sin(m_yaw) * cos(m_pitch);

	m_strafe_lx = (float)cos(m_yaw - M_PI_2);
	m_strafe_lz = (float)sin(m_yaw - M_PI_2);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(m_x, m_y, m_z, // Look from camera XYZ
		m_x + m_lx, m_y + m_ly, m_z + m_lz, // Look at this spot
		0.0,1.0,0.0); // Positive Y up vector

	glLightfv(GL_LIGHT0, GL_POSITION, light_p);
	glLoadIdentity();
	gluLookAt(m_x, m_y, m_z, // Look from camera XYZ
		m_x + m_lx, m_y + m_ly, m_z + m_lz, // Look at this spot
		0.0,1.0,0.0); // Positive Y up vector
}

void Camera::set_pos(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z =z;

	refresh();
}

void Camera::get_pos(float &x, float &y, float &z)
{
    x = m_x;
    y = m_y;
    z = m_z;
}

void Camera::get_direction(float &x, float &y, float &z)
{
    x = m_lx;
    y = m_ly;
    z = m_lz;
}

// TODO: movement speed independent of camera angle?
void Camera::move_forward(float incr)
{
    float lx = cos(m_yaw)*cos(m_pitch);

    // uncomment this to enable flying
    //float ly = sin(m_pitch);

    float ly = 0;
    float lz = sin(m_yaw)*cos(m_pitch);

	m_x = m_x + incr*lx;
	m_y = m_y + incr*ly;
	m_z = m_z + incr*lz;

	refresh();
}

void Camera::move_sideways(float incr)
{
	m_x = m_x + incr*m_strafe_lx;
	m_z = m_z + incr*m_strafe_lz;

	refresh();
}

void Camera::move_upwards(float incr)
{
	m_y = m_y + incr;

	refresh();
}

void Camera::adjust_yaw(float angle)
{
	m_yaw += angle;

	refresh();
}

void Camera::adjust_pitch(float angle)
{
    const float limit = (float) (89.0 * M_PI / 180.0);

	m_pitch -= angle;

    if(m_pitch < -limit) m_pitch = -limit;

    if(m_pitch > limit) m_pitch = limit;

	refresh();
}

void Camera::set_yaw(float angle)
{
	m_yaw = angle;

	refresh();
}

void Camera::set_pitch(float angle)
{
    m_pitch = angle;

    refresh();
}
