#pragma once

#ifndef SOLID_TEXTURE_H
#define SOLID_TEXTURE_H

#include"Texture.h"

class Solid_Texture : public Texture {
public:
	Solid_Texture() {};
	Solid_Texture(vec3 c):color(c) {};

	virtual vec3 value(double u, double v) const override {
		return color;
	}
private:
	vec3 color;
};
#endif // !SOLID_TEXTURE_H
