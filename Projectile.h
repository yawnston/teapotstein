#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include <array>
#include "Hitbox.h"

class Projectile
{
public:
	Hitbox hitbox;
	std::array<float, 3> location;
	std::array<float, 3> heading;

	static constexpr float movement_speed = 0.3f;
	static constexpr float size = 0.05f;
	static constexpr float max_lifetime = 150.0f; // lifetime is 150 frames

	void decrease_lifetime(float d)
	{
		lifetime_ -= d;
	}

	bool is_dead()
	{
		return lifetime_ <= 0.0f;
	}

	// move the projectile in a straight line by its speed increment
	void move()
	{
		for(size_t i = 0; i < 3; ++i)
		{
			location[i] += heading[i] * movement_speed;
		}
		hitbox.set_pos(location[0], location[1], location[2]);
	}

	Projectile(float x, float y, float z, float dx, float dy, float dz)
	{
		location = std::array<float, 3>();
		location[0] = x; location[1] = y; location[2] = z;
		heading = std::array<float, 3>();
		heading[0] = dx; heading[1] = dy; heading[2] = dz;
		lifetime_ = max_lifetime;
		hitbox = Hitbox(x, y, z, size);
	}

private:
	float lifetime_;


};

#endif
