/*
 
SPACEBAR - toggle FPS control
W,A,S,D - to move
mouse_button_callback - look around, inverted mouse_button_callback
left/right mouse_button_callback - fly up/down
ESC - quit

*/

//https://paroj.github.io/gltut/Positioning/Tut05%20Overlap%20and%20Depth%20Buffering.html
//http://nghiaho.com/?p=1613
//$ g++ main.cpp Camera.h Camera.cpp Hitbox.h -o test3 -Wall -lGL -lglut -lGLU

#define _USE_MATH_DEFINES

#include <GL/freeglut.h>
#include <iostream>
#include <cmath>
#include <random>
#include <ctime>
#include <vector>
#include <array>

#include "Camera.h"
#include "Hitbox.h"

using namespace std;

void display();
void reshape_callback (int w, int h);
void keyboard_callback(unsigned char key, int x, int y);
void keyboard_release_callback(unsigned char key, int x, int y);
void mouse_motion_callback(int x, int y);
void mouse_button_callback(int button, int state, int x, int y);
void movement_timer_callback(int value);
void idle_callback();

void init_enemies();
void init_projectiles();
void grid();
void enemy_movement(int value);
void enemy_direction(int value);
void projectile_movement(int value);
void refresh_fire_cooldown(int value);
void refresh_enemy_invulnerability(int value);
void damage_enemy(size_t enemy);
void check_projectile_collision();
void display_text(float x, float y, int r, int g, int b, const char* string);

Camera main_camera;
bool input_keys[256];
bool input_shift_down = false;
bool input_fps_mode = false;
int viewport_width = 0;
int viewport_height = 0;
bool input_mouse_left_down = false;
bool input_mouse_right_down = false;

GLfloat light_main_pos[] = {5.0f, 0.0f, 5.0f, 0.0f};
GLfloat light_ambient_pos[] = {0.8f, 0.4f, 0.0f, 1.0f};

const size_t enemy_count = 8;
float enemy_x[enemy_count];
float enemy_y[enemy_count];
float enemy_heading[enemy_count][2];
int enemy_health[enemy_count];
Hitbox enemy_hitboxes[enemy_count];
const float enemy_size = 0.5f;
const size_t enemy_max_health = 3;
bool enemy_invulnerability[enemy_count];
const size_t enemy_invulnerability_length = 500; // in miliseconds

vector<array<float,3>> projectile_location; // TODO: replace with deque to remove old projectiles? -> projectile age
vector<array<float,3>> projectile_heading;
vector<Hitbox> projectile_hitboxes;
const float projectile_size = 0.05f;
bool fire_cooldown = false;
const size_t fire_rate = 500; // in miliseconds

// Movement settings
const float g_translation_speed = 0.05f;
const float g_rotation_speed = (float)M_PI/180*0.2f;
const float g_enemy_speed = 0.05f;
const float g_projectile_speed = 0.2f;

int frame = 0, time_elapsed = 0, timebase = 0;
char fpscount_buffer[32];

int main (int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(640, 480);
    glutCreateWindow("hello");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, light_main_pos); 
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient_pos);
	glBegin(GL_LINES);
	glVertex3f(5.0, 0.0, 5.0);
	glVertex3f(5.0, 5.0, 5.0);
	glEnd();
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	glEnable (GL_CULL_FACE);
    glFrontFace (GL_CW);
    glCullFace (GL_FRONT); 
	
	init_enemies();
	init_projectiles();

    glutIgnoreKeyRepeat(1);

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape_callback);
    glutMouseFunc(mouse_button_callback);
    glutMotionFunc(mouse_motion_callback);
    glutPassiveMotionFunc(mouse_motion_callback);
    glutKeyboardFunc(keyboard_callback);
    glutKeyboardUpFunc(keyboard_release_callback);
    glutIdleFunc(idle_callback);

    glutTimerFunc(1, movement_timer_callback, 0);
	glutTimerFunc(50, enemy_movement, 2);
    glutTimerFunc(500, enemy_direction, 1);
	glutTimerFunc(50, projectile_movement, 3);

    glutMainLoop();

    return 0;
}

void init_enemies()
{
	srand((size_t)time(NULL)); // seed generator
	for(size_t i = 0; i < enemy_count; ++i)
	{
		enemy_x[i] = enemy_y[i] = (float)i;
		enemy_heading[i][0] = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
		enemy_heading[i][1] = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
		enemy_health[i] = enemy_max_health;
		enemy_hitboxes[i] = Hitbox(enemy_x[i], 0, enemy_y[i], enemy_size);
	}
}

void init_projectiles()
{
	projectile_location = vector<array<float,3>>();
	projectile_heading = vector<array<float,3>>();
}

void grid()
{
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glColor3f(1,1,1);

    for(int i=-50; i < 50; i++) {
        glBegin(GL_LINES);
        glVertex3f((float)i, -0.5f, -50.0f);
        glVertex3f((float)i, -0.5f, 50.0f);
        glEnd();
    }

    for(int i=-50; i < 50; i++) {
        glBegin(GL_LINES);
        glVertex3f(-50.0f, -0.5f, (float)i);
        glVertex3f(50.0f, -0.5f, (float)i);
        glEnd();
    }
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void trees()
{
	glColor3f(0.0f,1.0f,0.0f);

	for(int i = 1; i < 20; i += 3)
	{
		for(int j = 1; j < 20; j+= 3)
		{
			glPushMatrix();
			glColor3f(0.545f, 0.271f, 0.075f);
 
			glTranslatef((float)i, 0, (float)j);
			glutSolidCube(0.2f);
			glTranslatef(0, 0.2f, 0);
			glutSolidCube(0.24f);
			glTranslatef(0, 0.2f, 0);
			glutSolidCube(0.3f);
			glTranslatef(0.0f, 0.1f, 0.0f);
			glColor3f(0.0f, 0.7f, 0.0f);
			glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
			glutSolidCone(0.6, 0.5, 10, 10);
			glPopMatrix();
		}
	}
}

void enemy_direction(int value)
{
	for(size_t i = 0; i < enemy_count; ++i)
	{
		enemy_heading[i][0] = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
		enemy_heading[i][1] = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));	
	}
	glutTimerFunc(500, enemy_direction, 1);
}

void enemy_movement(int value)
{
	for(size_t i = 0; i < enemy_count; ++i)
	{
		enemy_x[i] += enemy_heading[i][0] * g_enemy_speed;
		enemy_y[i] += enemy_heading[i][1] * g_enemy_speed;
		enemy_hitboxes[i].set_pos(enemy_x[i], 0, enemy_y[i]);
	}
	glutTimerFunc(50, enemy_movement, 2);
}

void projectile_movement(int value)
{
	for(size_t i = 0; i < projectile_location.size(); ++i)
	{
		for(size_t j = 0; j < 3; ++j)
		{
			projectile_location[i][j] += projectile_heading[i][j] * g_projectile_speed;
		}
		projectile_hitboxes[i].set_pos(projectile_location[i][0], projectile_location[i][1], projectile_location[i][2]);
	}
	check_projectile_collision();
	glutTimerFunc(50, projectile_movement, 3);
}

void fire_projectile()
{
	if(fire_cooldown == true) return;
	float x,y,z;
	float hx, hy, hz;
	main_camera.get_pos(x,y,z); // TODO: are y and z switched? are coords correct?
	main_camera.get_direction(hx, hy, hz);
	array<float,3> p {0, 0, 0}; p[0] = x; p[1] = y; p[2] = z;
	array<float,3> h {0, 0, 0}; h[0] = hx; h[1] = hy; h[2] = hz;
	projectile_location.push_back(p);
	projectile_heading.push_back(h);
	projectile_hitboxes.push_back(Hitbox(x,y,z, projectile_size));

	fire_cooldown = true;
	glutTimerFunc(fire_rate, refresh_fire_cooldown, 4);
}

void refresh_fire_cooldown(int value)
{
	fire_cooldown = false;
}

void check_projectile_collision()
{
	for(auto&& projectile : projectile_hitboxes)
	{
		for(size_t j = 0; j < enemy_count; ++j)
		{
			bool result = projectile.check_collision(enemy_hitboxes[j]);
			if(result) damage_enemy(j);
		}
	}
}

void damage_enemy(size_t enemy)
{
	if(enemy_invulnerability[enemy]) return; // don't do anything if the enemy is recently damaged
	if(enemy_health[enemy] > 0) enemy_health[enemy]--;
	enemy_invulnerability[enemy] = true;
	glutTimerFunc(enemy_invulnerability_length, refresh_enemy_invulnerability, enemy);
}

// value is the number of the enemy to refresh
void refresh_enemy_invulnerability(int value)
{
	enemy_invulnerability[value] = false;
}

void show_enemies()
{
	for(size_t i = 0; i < enemy_count; ++i)
	{
		if(enemy_health[i] == 3) glColor3f(0.9f, 0.1f, 0.1f); 
		if(enemy_health[i] == 2) glColor3f(0.6f, 0.1f, 0.1f); 
		if(enemy_health[i] == 1) glColor3f(0.3f, 0.1f, 0.1f); 
		if(enemy_health[i] <= 0) glColor3f(0.0f, 0.1f, 0.1f); 

		glPushMatrix();
		glTranslatef(enemy_x[i], 0, enemy_y[i]);
		glutSolidTeapot(enemy_size);
		glPopMatrix();

		// show enemy hitboxes
		glColor3f(0.0f, 0.9f, 0.1f);
		glPushMatrix();
		float x, y, z;
		enemy_hitboxes[i].get_pos(x,y,z);
		glTranslatef(x,y,z);
		glutWireSphere(enemy_size, 10, 10);
		glPopMatrix();

	}
}

void show_projectiles()
{

	for(size_t i = 0; i < projectile_location.size(); ++i)
	{
		glColor3f(0.2f, 0.1f, 0.8f);
		// show projectiles as teapots
		glPushMatrix();
		glTranslatef(projectile_location[i][0], projectile_location[i][1], projectile_location[i][2]);
		glutSolidTeapot(0.05);
		glPopMatrix();

		// show projectile hitboxes
		glColor3f(0.0f, 0.9f, 0.1f);
		glPushMatrix();
		float x, y, z;
		projectile_hitboxes[i].get_pos(x,y,z);
		glTranslatef(x,y,z);
		glutWireSphere(projectile_size, 10, 10);
		glPopMatrix();
	}

}

void display_text(float x, float y, int r, int g, int b, const char* string)
{
	const char* c;
	float pos = x;
	int spacing = 10;

	for (c = string; *c != '\0'; ++c)
	{

		glRasterPos2f(pos, y);
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
		pos = pos + glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c) + spacing;
	}
}

void set_ortho_projection()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, viewport_width, viewport_height, 0);
	glMatrixMode(GL_MODELVIEW);
}

void restore_perspective_projection()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void display (void) {
    glClearColor (0.0,0.0,0.0,1.0); //clear the screen to black
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the color buffer and the depth buffer
    glLoadIdentity();

	main_camera.refresh();

    glColor3f(0,1,0);

    //glutSolidTeapot(0.5);
    grid();
    trees();
	show_enemies();
	show_projectiles();
	
	//calculate the frames per second
	frame++;

	//get the current time
	size_t currenttime = glutGet(GLUT_ELAPSED_TIME);
	//check if a second has passed
	if (currenttime - timebase > 1000)
	{
		sprintf_s(fpscount_buffer, "FPS: %4.2f", frame*1000.0 / (currenttime - timebase));
		timebase = currenttime;
		frame = 0;
	}

	set_ortho_projection();

	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	display_text(5, 30, 100, 30, 100, fpscount_buffer);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	restore_perspective_projection();

	glFlush();
    glutSwapBuffers(); //swap the buffers
}

void reshape_callback (int w, int h) {
    viewport_width = w;
    viewport_height = h;

    glViewport (0, 0, (GLsizei)w, (GLsizei)h); //set the viewport to the current window specifications
    glMatrixMode (GL_PROJECTION); //set the matrix to projection

    glLoadIdentity ();
    gluPerspective (60, (GLfloat)w / (GLfloat)h, 0.1 , 100.0); //set the perspective (angle of sight, width, height, ,depth)

    glMatrixMode (GL_MODELVIEW); //set the matrix back to model
}

void keyboard_callback(unsigned char key, int x, int y)
{
    if(key == 27) {
        exit(0);
    }

    if(key == ' ') {
        input_fps_mode = !input_fps_mode;

        if(input_fps_mode) {
            glutSetCursor(GLUT_CURSOR_NONE);
            glutWarpPointer(viewport_width/2, viewport_height/2);
        }
        else {
            glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
        }
    }

    if(glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
        input_shift_down = true;
    }
    else {
        input_shift_down = false;
    }

    input_keys[key] = true;
}

void keyboard_release_callback(unsigned char key, int x, int y)
{
    input_keys[key] = false;
}

void movement_timer_callback(int value)
{
    if(input_fps_mode) {
        if(input_keys['w'] || input_keys['W']) {
            main_camera.move_forward(g_translation_speed);
        }
        if(input_keys['s'] || input_keys['S']) {
            main_camera.move_forward(-g_translation_speed);
        }
        if(input_keys['a'] || input_keys['A']) {
            main_camera.move_sideways(g_translation_speed);
        }
        if(input_keys['d'] || input_keys['D']) {
            main_camera.move_sideways(-g_translation_speed);
        }
        if(input_mouse_left_down) {
			fire_projectile();
        }
        if(input_mouse_right_down) {
            main_camera.move_upwards(g_translation_speed);
        }
    }

    glutTimerFunc(1, movement_timer_callback, 0);
}

void idle_callback()
{
    display();
}

void mouse_button_callback(int button, int state, int x, int y)
{
    if(state == GLUT_DOWN) {
        if(button == GLUT_LEFT_BUTTON) {
            input_mouse_left_down = true;
        }
        else if(button == GLUT_RIGHT_BUTTON) {
            input_mouse_right_down = true;
        }
    }
    else if(state == GLUT_UP) {
        if(button == GLUT_LEFT_BUTTON) {
            input_mouse_left_down = false;
        }
        else if(button == GLUT_RIGHT_BUTTON) {
            input_mouse_right_down = false;
        }
    }
}

void mouse_motion_callback(int x, int y)
{
    // This variable is hack to stop glutWarpPointer from triggering an event callback to mouse_button_callback(...)
    // This avoids it being called recursively and hanging up the event loop
    static bool just_warped = false;

    if(just_warped) {
        just_warped = false;
        return;
    }

    if(input_fps_mode) {
        int dx = x - viewport_width/2;
        int dy = y - viewport_height/2;

        if(dx) {
            main_camera.adjust_yaw(g_rotation_speed*dx);
        }

        if(dy) {
            main_camera.adjust_pitch(g_rotation_speed*dy);
        }

        glutWarpPointer(viewport_width/2, viewport_height/2);

        just_warped = true;
    }
}
