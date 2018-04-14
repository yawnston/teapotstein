#ifndef __CAMERA_H__
#define __CAMERA_H__


class Camera
{
public:
	Camera() { init(); }
	~Camera(){}

	void init();
	void refresh();
	void set_pos(float x, float y, float z);
    void get_pos(float &x, float &y, float &z);
    void get_direction(float &x, float &y, float &z);
	void set_yaw(float angle);
	void set_pitch(float angle);

	void move_forward(float incr);
	void move_sideways(float incr);
	void move_upwards(float incr);
	void adjust_yaw(float angle);
	void adjust_pitch(float angle);

private:
	float m_x, m_y, m_z;   // Position
    float m_lx, m_ly, m_lz; // Direction vector of where we are looking at
	float m_yaw, m_pitch; // Various rotation angles
	float m_strafe_lx, m_strafe_lz; // Always 90 degree to direction vector
};

#endif
