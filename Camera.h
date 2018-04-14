#ifndef __CAMERA_H__
#define __CAMERA_H__


class Camera
{
public:
	Camera() { init(); }
	~Camera() {}

	void init();
	void refresh();
	void set_pos(float x, float y, float z);
	void get_pos(float& x, float& y, float& z);
	void get_direction(float& x, float& y, float& z);
	void set_yaw(float angle);
	void set_pitch(float angle);

	void move_forward(float incr);
	void move_sideways(float incr);
	void move_upwards(float incr);
	void adjust_yaw(float angle);
	void adjust_pitch(float angle);

private:
	float pos_x, pos_y, pos_z;   // camera position
	float dir_x, dir_y, dir_z; // camera direction vector
	float yaw, pitch; // rotation angles
	float dir_strafe_x, dir_strafe_z; // direction vectors for strafing
};

#endif
