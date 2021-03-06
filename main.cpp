#define _USE_MATH_DEFINES
#undef HITBOX_DEBUG

#include <GL/freeglut.h>
#include <iostream>
#include <cmath>
#include <random>
#include <ctime>
#include <vector>
#include <array>
#include <deque>
#include <string>

#include "Camera.h"
#include "Hitbox.h"
#include "Projectile.h"
#include "Particle.h"

using namespace std;

void display();
void reshape_callback(int w, int h);
void keyboard_callback(unsigned char key, int x, int y);
void keyboard_release_callback(unsigned char key, int x, int y);
void mouse_motion_callback(int x, int y);
void mouse_button_callback(int button, int state, int x, int y);
void movement_timer_callback(int value);
void idle_callback();

void init_enemies();
void init_projectiles();
void init_particles();
void init_floor();
void grid();
void floor();
void enemy_movement(int value);
void enemy_direction(int value);
void enemy_shooting(int value);
void enemy_fire_projectile(int value);
void projectile_movement(int value);
void refresh_fire_cooldown(int value);
void refresh_enemy_invulnerability(int value);
void refresh_enemy_fire_cooldown(int value);
void refresh_player_invulnerability(int value);
void damage_player();
void game_over();

void damage_enemy(size_t enemy);
void check_enemy_touch_collision();
void check_projectile_collision();
void check_enemy_projectile_collision();
void display_text(float x, float y, unsigned char r, unsigned char g, unsigned char b, const char* string);
void set_ortho_projection();
void restore_perspective_projection();

Camera main_camera;
bool input_keys[256];
bool input_shift_down = false;
bool input_fps_mode = false;
int viewport_width = 0;
int viewport_height = 0;
bool input_mouse_left_down = false;
bool input_mouse_right_down = false;

GLfloat light_main_pos[] = { 5.0f, 0.0f, 5.0f, 0.0f };
GLfloat light_ambient_pos[] = { 0.8f, 0.4f, 0.0f, 1.0f };

const int floor_size = 15;
float floor_color[floor_size][floor_size][2];

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

deque<Projectile> enemy_projectiles;
bool enemy_fire_cooldown[enemy_count];

deque<Projectile> active_projectiles;
bool fire_cooldown = false;
const size_t fire_rate = 500; // in miliseconds

deque<Particle> active_particles;
deque<Projectile> active_sparks;
const float spark_lifetime = 20;

bool game_over_state = false;
const float player_max_health = 150;
float player_health = player_max_health;
int player_score = 0;
Hitbox player_hitbox;
const float player_hitbox_size = 0.25f;
bool player_invulnerability;
const size_t player_invulnerability_length = 500; // in miliseconds

// Movement settings
const float player_movement_speed = 0.05f;
const float player_turn_speed = (float)M_PI / 180 * 0.2f;
const float enemy_movement_speed = 0.01f;

int frame = 0, time_elapsed = 0, timebase = 0;
char fpscount_buffer[32];

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1280, 720);
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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	init_enemies();
	init_projectiles();
	init_particles();
	init_floor();

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

	// Timer callback values:
	// 0 - movement timer
	// 1 - enemy direction
	// 2 - enemy movement
	// 3 - projectile movement
	// 4 - fire cooldown
	// 5 - enemy shooting
	// 6 - player invulnerability

	glutTimerFunc(1, movement_timer_callback, 0);
	glutTimerFunc(5, enemy_movement, 2);
	glutTimerFunc(500, enemy_direction, 1);
	glutTimerFunc(5, projectile_movement, 3);
	glutTimerFunc(50, enemy_shooting, 5);
	glutMainLoop();

	return 0;
}

void init_enemies()
{
	srand((size_t)time(NULL)); // seed generator
	for (size_t i = 0; i < enemy_count; ++i)
	{
		enemy_x[i] = -floor_size + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (floor_size * 2)));
		enemy_y[i] = -floor_size + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (floor_size * 2)));
		enemy_heading[i][0] = -1 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2)));
		enemy_heading[i][1] = -1 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2)));
		enemy_health[i] = enemy_max_health;
		enemy_hitboxes[i] = Hitbox(enemy_x[i], 0, enemy_y[i], enemy_size);
		enemy_fire_cooldown[i] = true;
		glutTimerFunc((size_t)(2000 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (5000))))
			, refresh_enemy_fire_cooldown, i + 64);
	}
	enemy_projectiles = deque<Projectile>();
}

void init_projectiles()
{	
	active_projectiles = deque<Projectile>();
}

void init_particles()
{
	active_particles = deque<Particle>();
	active_sparks = deque<Projectile>();
}

void init_player()
{
	float x, y, z;
	main_camera.get_pos(x, y, z);
	player_hitbox = Hitbox(x, y, z, player_hitbox_size);
	player_health = player_max_health;
	player_invulnerability = false;
}

void init_floor()
{
	float rand1, rand2;

	for (int i = 0; i < floor_size; ++i)
	{
		for (int j = 0; j < floor_size; ++j)
		{
			rand1 = (float)((rand() % 4000 - 2000) / 20000.0);
			rand2 = (float)((rand() % 4000 - 2000) / 20000.0);
			floor_color[i][j][0] = rand1 + 0.4f;
			floor_color[i][j][1] = rand2 + 0.4f;
		}
	}
}

// debugging grid to help visualize coordinates
void grid()
{
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glColor3f(1, 1, 1);

	for (int i = -50; i < 50; i++)
	{
		glBegin(GL_LINES);
		glVertex3f((float)i, -0.5f, -50.0f);
		glVertex3f((float)i, -0.5f, 50.0f);
		glEnd();
	}

	for (int i = -50; i < 50; i++)
	{
		glBegin(GL_LINES);
		glVertex3f(-50.0f, -0.5f, (float)i);
		glVertex3f(50.0f, -0.5f, (float)i);
		glEnd();
	}
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void floor()
{
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glFrontFace(GL_CW);

	// real floor size is 2*floor_size by 2*floor_size, centered around (0,-0.5,0)
	glBegin(GL_QUADS);
	for (int i = 0; i < floor_size; ++i)
	{
		for (int j = 0; j < floor_size; ++j)
		{
			glColor3f(floor_color[i][j][0], floor_color[i][j][1], 0.14f);

			glVertex3f((float)i - 1, -0.5f, (float)j);
			glVertex3f((float)i - 1, -0.5f, (float)j - 1);
			glVertex3f((float)i, -0.5f, (float)j - 1);
			glVertex3f((float)i, -0.5f, (float)j);

			glVertex3f((float)i - floor_size - 1, -0.5f, (float)j);
			glVertex3f((float)i - floor_size - 1, -0.5f, (float)j - 1);
			glVertex3f((float)i - floor_size, -0.5f, (float)j - 1);
			glVertex3f((float)i - floor_size, -0.5f, (float)j);

			glVertex3f((float)i - 1, -0.5f, (float)j - floor_size);
			glVertex3f((float)i - 1, -0.5f, (float)j - floor_size - 1);
			glVertex3f((float)i, -0.5f, (float)j - floor_size - 1);
			glVertex3f((float)i, -0.5f, (float)j - floor_size);

			glVertex3f((float)i - floor_size - 1, -0.5f, (float)j - floor_size);
			glVertex3f((float)i - floor_size - 1, -0.5f, (float)j - floor_size - 1);
			glVertex3f((float)i - floor_size, -0.5f, (float)j - floor_size - 1);
			glVertex3f((float)i - floor_size, -0.5f, (float)j - floor_size);
		}
	}
	glEnd();
	glEnable(GL_LIGHTING);
	glPopMatrix();
	glFrontFace(GL_CCW);
}

void trees()
{
	glColor3f(0.0f, 1.0f, 0.0f);

	for (int i = -9; i < 10; i += 3)
	{
		for (int j = -9; j < 10; j += 3)
		{
			glPushMatrix();
			glColor3f(0.545f, 0.271f, 0.075f);

			glTranslatef((float)i, -0.5f, (float)j);
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

void skybox()
{
	glPushMatrix();
	glLoadIdentity();
	glFrontFace(GL_CW);

	float x, y, z;
	main_camera.get_direction(x, y, z);
	gluLookAt(
		0, 0, 0,
		x, y, z,
		0, 1, 0);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);

	// Render the front quad
	glColor4f(0.9f, 0.6f, 0.6f, 1.0f);
	glBegin(GL_QUADS);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glEnd();

	// Render the left quad
	glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
	glBegin(GL_QUADS);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glEnd();

	// Render the back quad
	glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
	glBegin(GL_QUADS);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glEnd();

	// Render the right quad
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
	glBegin(GL_QUADS);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glEnd();

	// Render the top quad
	glColor4f(0.3f, 0.3f, 0.3f, 1.0f);
	glBegin(GL_QUADS);
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glEnd();

	// Render the bottom quad
	glColor4f(0.1f, 0.1f, 0.1f, 1.0f);
	glBegin(GL_QUADS);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glEnd();

	glPopAttrib();
	glPopMatrix();
	glFrontFace(GL_CCW);
}

void enemy_direction(int value)
{
	if(value != 1) return;
	for (size_t i = 0; i < enemy_count; ++i)
	{
		/*
		enemy_heading[i][0] = -1 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2)));
		enemy_heading[i][1] = -1 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2)));
		*/
		float px, py, pz;
		main_camera.get_pos(px, py, pz);
		float dx, dz;
		dx = px - enemy_x[i];
		dz = pz - enemy_y[i];
		// normalize direction vector
		float vec_len = sqrt(dx * dx + dz * dz);
		float nx, nz;
		nx = dx / vec_len;
		nz = dz / vec_len;
		enemy_heading[i][0] = nx;
		enemy_heading[i][1] = nz;
	}
	glutTimerFunc(500, enemy_direction, 1);
}

void enemy_movement(int value)
{
	if(value != 2 || game_over_state) return;
	for (size_t i = 0; i < enemy_count; ++i)
	{
		enemy_x[i] += enemy_heading[i][0] * enemy_movement_speed;
		enemy_y[i] += enemy_heading[i][1] * enemy_movement_speed;
		enemy_hitboxes[i].set_pos(enemy_x[i], 0, enemy_y[i]);
	}
	check_enemy_touch_collision();
	glutTimerFunc(5, enemy_movement, 2);
}

void enemy_shooting(int value)
{
	if (value != 5 || game_over_state) return;
	for (int i = 0; i < enemy_count; ++i)
	{
		if (!enemy_fire_cooldown[i]) enemy_fire_projectile(i);
	}
	glutTimerFunc(50, enemy_shooting, 5);
}

void enemy_fire_projectile(int enemy)
{
	// calculate heading from enemy position and player position
	float px, py, pz;
	main_camera.get_pos(px, py, pz);
	float dx, dy, dz;
	dx = px - enemy_x[enemy];
	dy = 0;
	dz = pz - enemy_y[enemy];
	// normalize direction vector
	float vec_len = sqrt(dx * dx + dz * dz);
	float nx, ny, nz;
	nx = dx / vec_len;
	ny = dy / vec_len;
	nz = dz / vec_len;
	
	enemy_projectiles.push_back(Projectile(enemy_x[enemy], 0.0f, enemy_y[enemy], nx, ny, nz));
	enemy_fire_cooldown[enemy] = true;
	// enemies fire randomly between 5000 and 10000 ms
	glutTimerFunc(5000 + (size_t)(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (5000))))
		, refresh_enemy_fire_cooldown, enemy + 64);
}

void particle_update()
{
	for (auto&& p : active_particles)
	{
		p.decrease_lifetime(1.0f);
	}
	for (auto&& s : active_sparks)
	{
		s.move();
		s.decrease_lifetime(1.0f);
	}
	while (true)
	{
		if (active_particles.size() == 0 || !(active_particles.front().is_dead())) break;
		active_particles.pop_front();
	}
	while (true)
	{
		if (active_sparks.size() == 0 || !(active_sparks.front().is_dead())) break;
		active_sparks.pop_front();
	}
}

void projectile_floor_collision(float x, float z)
{
	active_sparks.emplace_back(x, -0.5f, z,
		0.707f, 0.707f, 0.0f);
	active_sparks.back().set_lifetime(spark_lifetime);
	active_sparks.emplace_back(x, -0.5f, z,
		-0.707f, 0.707f, 0.0f);
	active_sparks.back().set_lifetime(spark_lifetime);
	active_sparks.emplace_back(x, -0.5f, z,
		0.0f, 0.707f, 0.707f);
	active_sparks.back().set_lifetime(spark_lifetime);
	active_sparks.emplace_back(x, -0.5f, z,
		0.0f, 0.707f, -0.707f);
	active_sparks.back().set_lifetime(spark_lifetime);
}

void projectile_movement(int value)
{
	if(value != 3 || game_over_state) return;

	for(auto&& p : active_projectiles)
	{
		// spawn a bullet trail particle
		active_particles.emplace_back(p.location[0], p.location[1], p.location[2],
			0.03f, 5.0f,
			0.2f, 0.1f, 0.8f);
		p.move();
		// if the bullet hit the ground, spawn some special effects
		if (p.location[1] <= -0.5f && !p.hit_floor)
		{
			p.hit_floor = true;
			projectile_floor_collision(p.location[0], p.location[2]);
		}
		p.decrease_lifetime(1.0f);
	}
	for(auto&& p : enemy_projectiles)
	{
		p.move();
		p.decrease_lifetime(1.0f);
	}


	// remove dead projectiles from the active deque (oldest projectiles are always at the front)
	while(true)
	{
		if(active_projectiles.size() == 0 || !(active_projectiles.front().is_dead())) break;
		active_projectiles.pop_front();
	}
	while(true)
	{
		if(enemy_projectiles.size() == 0 || !(enemy_projectiles.front().is_dead())) break;
		enemy_projectiles.pop_front();
	}

	particle_update();
	check_projectile_collision();
	check_enemy_projectile_collision();
	glutTimerFunc(5, projectile_movement, 3);
}

void fire_projectile()
{
	if (fire_cooldown == true) return;
	float x, y, z;
	float hx, hy, hz;
	main_camera.get_pos(x, y, z);
	main_camera.get_direction(hx, hy, hz);
	
	active_projectiles.push_back(Projectile(x,y,z,hx,hy,hz));

	fire_cooldown = true;
	glutTimerFunc(fire_rate, refresh_fire_cooldown, 4);
}

void refresh_fire_cooldown(int value)
{
	if(value != 4 || game_over_state) return;	
	fire_cooldown = false;
}

void refresh_enemy_fire_cooldown(int value)
{
	if (value < 64 || value >= (64 + enemy_count)) return;
	enemy_fire_cooldown[value - 64] = false;
}

void check_enemy_touch_collision()
{
	for (auto&& e : enemy_hitboxes)
	{
		bool result = e.check_collision(player_hitbox);
		if (result) damage_player();
	}
}

void check_projectile_collision()
{
	for(auto&& p : active_projectiles)
	{
		for (size_t j = 0; j < enemy_count; ++j)
		{
			bool result = p.hitbox.check_collision(enemy_hitboxes[j]);
			if (result) damage_enemy(j);
		}
	}
}

void check_enemy_projectile_collision()
{
	for(auto&& p : enemy_projectiles)
	{
		bool result = p.hitbox.check_collision(player_hitbox);
		if (result) damage_player();
	}
}

// despawn the given enemy and replace it with a new one
void destroy_enemy(size_t enemy)
{
	srand((size_t)time(NULL)); // seed generator
	enemy_x[enemy] = -floor_size + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (floor_size * 2)));
	enemy_y[enemy] = -floor_size + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (floor_size * 2)));
	enemy_heading[enemy][0] = -1 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2)));
	enemy_heading[enemy][1] = -1 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2)));
	enemy_health[enemy] = enemy_max_health;
	enemy_hitboxes[enemy] = Hitbox(enemy_x[enemy], 0, enemy_y[enemy], enemy_size);
	++player_score;
}

void damage_enemy(size_t enemy)
{
	if (enemy_invulnerability[enemy]) return; // don't do anything if the enemy is recently damaged
	if (enemy_health[enemy] > 0) enemy_health[enemy]--;
	if(enemy_health[enemy] == 0)
	{
		destroy_enemy(enemy);
	}
	enemy_invulnerability[enemy] = true;
	glutTimerFunc(enemy_invulnerability_length, refresh_enemy_invulnerability, enemy + 128);
}

// if the player is out of bounds, they lose
void check_player_oob()
{
	float x, y, z; main_camera.get_pos(x, y, z);
	bool result = (x < -floor_size - 1 || x > floor_size)
		|| (z < -floor_size - 1 || z > floor_size);
	if (result) game_over();
}

void game_over()
{
	game_over_state = true;
	input_fps_mode = false;
	glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
}

void damage_player()
{
	if (player_invulnerability) return;
	if (player_health > 0.0f) player_health -= 25.0f;
	if (player_health <= 0.0f) game_over();
	player_invulnerability = true;
	glutTimerFunc(player_invulnerability_length, refresh_player_invulnerability, 6);
}

void refresh_player_invulnerability(int value)
{
	if (value != 6) return;
	player_invulnerability = false;
}

// value is the number of the enemy to refresh
void refresh_enemy_invulnerability(int value)
{
	if (value < 128 || value >= (128 + enemy_count)) return;
	enemy_invulnerability[value - 128] = false;
}

void show_enemies()
{
	glFrontFace(GL_CW);
	for (size_t i = 0; i < enemy_count; ++i)
	{
		if (enemy_health[i] == 3) glColor3f(0.9f, 0.1f, 0.1f);
		if (enemy_health[i] == 2) glColor3f(0.6f, 0.1f, 0.1f);
		if (enemy_health[i] == 1) glColor3f(0.3f, 0.1f, 0.1f);
		if (enemy_health[i] <= 0) glColor3f(0.0f, 0.1f, 0.1f);

		glPushMatrix();
		glTranslatef(enemy_x[i], 0, enemy_y[i]);
		glutSolidTeapot(enemy_size);
		glPopMatrix();

#ifdef HITBOX_DEBUG
		// show enemy hitboxes
		glColor3f(0.0f, 0.9f, 0.1f);
		glPushMatrix();
		float x, y, z;
		enemy_hitboxes[i].get_pos(x, y, z);
		glTranslatef(x, y, z);
		glutWireSphere(enemy_size, 10, 10);
		glPopMatrix();
#endif
	}

#ifdef HITBOX_DEBUG
	// show player hitbox
	glColor3f(0.0f, 0.9f, 0.1f);
	glPushMatrix();
	float x, y, z;
	player_hitbox.get_pos(x, y, z);
	glTranslatef(x, y, z);
	glutWireSphere(player_hitbox_size, 10, 10);
	glPopMatrix();
	glFrontFace(GL_CCW);
#endif
}

void show_projectiles()
{
	for(auto&& p : active_projectiles)
	{
		glColor3f(0.2f, 0.1f, 0.8f);
		// show projectiles as teapots
		glPushMatrix();
		glTranslatef(p.location[0], p.location[1], p.location[2]);
		glutSolidTeapot(0.05);
		glPopMatrix();

#ifdef HITBOX_DEBUG
		// show projectile hitboxes
		glColor3f(0.0f, 0.9f, 0.1f);
		glPushMatrix();
		float x, y, z;
		p.hitbox.get_pos(x, y, z);
		glTranslatef(x, y, z);
		glutWireSphere(Projectile::size, 10, 10);
		glPopMatrix();
#endif
	}

	for(auto&& p : enemy_projectiles)
	{
		glColor3f(0.8f, 0.2f, 0.1f);
		// show projectiles as teapots
		glPushMatrix();
		glTranslatef(p.location[0], p.location[1], p.location[2]);
		glutSolidTeapot(0.05);
		glPopMatrix();

#ifdef HITBOX_DEBUG
		// show projectile hitboxes
		glColor3f(0.0f, 0.9f, 0.1f);
		glPushMatrix();
		float x, y, z;
		p.hitbox.get_pos(x, y, z);
		glTranslatef(x, y, z);
		glutWireSphere(Projectile::size, 10, 10);
		glPopMatrix();
#endif
	}
}

void show_particles()
{
	for (auto&& p : active_particles)
	{
		if (p.is_dead()) continue;
		glColor4f(p.color[0], p.color[1], p.color[2], p.get_alpha());
		glPushMatrix();
		glTranslatef(p.location[0], p.location[1], p.location[2]);
		glutSolidTeapot(p.size);
		glPopMatrix();
	}

	for (auto&& p : active_sparks)
	{
		glColor3f(0.992f, 0.952f, 0.247f); // yellow
		glPushMatrix();
		glTranslatef(p.location[0], p.location[1], p.location[2]);
		glutSolidTeapot(0.01);
		glPopMatrix();
	}
}

void display_text(float x, float y, unsigned char r, unsigned char g, unsigned char b, const char* string)
{
	const char* c;
	float pos = x;
	int spacing = 10;

	glColor3ub(r, g, b);

	for (c = string; *c != '\0'; ++c)
	{
		glRasterPos2f(pos, y);
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
		pos = pos + glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c) + spacing;
	}
}

void display_hud()
{
	set_ortho_projection();

	glDisable(GL_CULL_FACE);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_LIGHTING);

	const int numDiv = 15;
	const float sep = 2.0f;
	const float barHeight = 75.0f / (float)numDiv;
	glBegin(GL_QUADS);
	glColor3f(1, 0, 0);
	// draw player HP bars
	for (float i = 0; i < player_health; i += (sep + barHeight))
	{
		glVertex2f((float)(viewport_width - 80), (float)(i));
		glVertex2f((float)(viewport_width), (float)(i));
		glVertex2f((float)(viewport_width), (float)(i + barHeight));
		glVertex2f((float)(viewport_width - 80), (float)(i + barHeight));
	}
	glEnd();
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	restore_perspective_projection();
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

void display()
{
	glClearColor(0.0, 0.0, 0.0, 1.0); //black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	main_camera.refresh();

	skybox();

	glColor3f(0, 1, 0);

	//grid();
	floor();
	trees();
	show_enemies();
	show_projectiles();
	show_particles();
	display_hud();

	//calculate the frames per second
	frame++;
	size_t currenttime = glutGet(GLUT_ELAPSED_TIME);
	//check if a second has passed
	if (currenttime - timebase > 1000)
	{
		sprintf(fpscount_buffer, "FPS: %4.2f", frame*1000.0 / (currenttime - timebase));
		timebase = currenttime;
		frame = 0;
	}

	set_ortho_projection();

	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	display_text(5.0f, 30.0f, 255, 255, 51, fpscount_buffer);
	display_text(viewport_width / 2.0f, 30.0f, 255, 255, 51, ("SCORE: " + to_string(player_score)).c_str());
	if(game_over_state) display_text(viewport_width / 2.0f, viewport_height / 2.0f, 255, 255, 51, ("GAME OVER!"));
	glEnable(GL_LIGHTING);
	glPopMatrix();

	restore_perspective_projection();

	glFlush();
	glutSwapBuffers();
}

void reshape_callback(int w, int h)
{
	viewport_width = w;
	viewport_height = h;

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	gluPerspective(75, (GLfloat)w / (GLfloat)h, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
}

void keyboard_callback(unsigned char key, int x, int y)
{
	if (key == 27)
	{
		exit(0);
	}

	if (key == ' ')
	{
		input_fps_mode = !input_fps_mode;

		if (input_fps_mode)
		{
			glutSetCursor(GLUT_CURSOR_NONE);
			glutWarpPointer(viewport_width / 2, viewport_height / 2);
		}
		else
		{
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		}
	}

	if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
	{
		input_shift_down = true;
	}
	else
	{
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
	if(value != 0 || game_over_state) return;
	float x, y, z;
	if (input_fps_mode)
	{
		if (input_keys['w'] || input_keys['W'])
		{
			main_camera.move_forward(player_movement_speed);
		}
		if (input_keys['s'] || input_keys['S'])
		{
			main_camera.move_forward(-player_movement_speed);
		}
		if (input_keys['a'] || input_keys['A'])
		{
			main_camera.move_sideways(player_movement_speed);
		}
		if (input_keys['d'] || input_keys['D'])
		{
			main_camera.move_sideways(-player_movement_speed);
		}
		if (input_mouse_left_down)
		{
			fire_projectile();
		}
		if (input_mouse_right_down)
		{
			main_camera.move_upwards(player_movement_speed);
		}
	}
	main_camera.get_pos(x, y, z);
	player_hitbox.set_pos(x, y, z);
	check_player_oob();

	glutTimerFunc(1, movement_timer_callback, 0);
}

void idle_callback()
{
	display();
}

void mouse_button_callback(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			input_mouse_left_down = true;
		}
		else if (button == GLUT_RIGHT_BUTTON)
		{
			input_mouse_right_down = true;
		}
	}
	else if (state == GLUT_UP)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			input_mouse_left_down = false;
		}
		else if (button == GLUT_RIGHT_BUTTON)
		{
			input_mouse_right_down = false;
		}
	}
}

void mouse_motion_callback(int x, int y)
{
	// prevents infinite loop with glutWarpPointer
	static bool just_warped = false;

	if (just_warped)
	{
		just_warped = false;
		return;
	}

	if (input_fps_mode)
	{
		int dx = x - viewport_width / 2;
		int dy = y - viewport_height / 2;

		if (dx)
		{
			main_camera.adjust_yaw(player_turn_speed*dx);
		}

		if (dy)
		{
			main_camera.adjust_pitch(player_turn_speed*dy);
		}

		// teleport the invisible cursor to the middle of the screen to prevent it from leaving the window in FPS mode
		glutWarpPointer(viewport_width / 2, viewport_height / 2);

		just_warped = true;
	}
}
