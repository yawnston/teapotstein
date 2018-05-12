#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include <array>

class Particle
{
public:
	std::array<float, 3> location;
	std::array<float, 3> color;
	float size;

	void decrease_lifetime(float d)
	{
		lifetime_ -= d;
	}

	bool is_dead()
	{
		return lifetime_ <= 0.0f;
	}

	float get_alpha()
	{
		float res = lifetime_ / max_lifetime_;
		return res >= 0.0f ? res : 0.0f;
	}

	Particle(float x, float y, float z, float s, float lifetime, float r, float g, float b)
	{
		location = std::array<float, 3>();
		location[0] = x; location[1] = y; location[2] = z;
		color = std::array<float, 3>();
		color[0] = r; color[1] = g; color[2] = b;
		lifetime_ = max_lifetime_ = lifetime;
		size = s;
	}

private:
	float lifetime_;
	float max_lifetime_;

};

#endif
