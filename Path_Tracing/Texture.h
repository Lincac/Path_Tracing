#pragma once

#ifndef TEXTURE_H
#define TEXTURE_H

#include"vec3.h"

class Texture {
public:
	virtual vec3 value(double u, double v) const = 0;
};

#endif // !TEXTURE_H
