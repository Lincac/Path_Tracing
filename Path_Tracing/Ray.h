#pragma once

#ifndef RAY_H
#define RAY_H

#include"vec3.h"

class Ray {
public:
	vec3 origin;
	vec3 direction;

	Ray() {};
	Ray(const vec3& origin, const vec3& direction) {
		this->origin = origin;
		this->direction = direction;
	}
};

#endif // !RAY_H
