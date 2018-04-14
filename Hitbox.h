#ifndef __HITBOX_H__
#define __HITBOX_H__

#include <cmath>

// Uses sphere collision
class Hitbox
{
public:
	bool check_collision(const Hitbox& hb) const
	{
		float dx = this->x - hb.x;
		float dy = this->y - hb.y;
		float dz = this->z - hb.z;
		float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

		if (distance <= this->radius + hb.radius) return true;
		return false;
	}

	void set_pos(float a, float b, float c)
	{
		x = a; y = b; z = c;
	}

	void get_pos(float& a, float& b, float& c)
	{
		a = x; b = y; c = z;
	}

	Hitbox()
	{
		set_pos(0, 0, 0);
	}

	Hitbox(float a, float b, float c, float r)
	{
		set_pos(a, b, c);
		radius = r;
	}
private:
	float x, y, z;
	float radius;
};

#endif
