#pragma once

#ifndef AABB_H
#define AABB_H

#include"Ray.h"

class AABB {
public:
	AABB() {};
	AABB(vec3 a,vec3 b) : _min(a),_max(b) {};

	bool hit_aabb(const Ray& ray) const;

	vec3 min_() { return _min; };
	vec3 max_() { return _max; };

private:
	vec3 _min;
	vec3 _max;
};

bool AABB::hit_aabb(const Ray& ray) const {
	vec3 t0 = (_min - ray.origin) / ray.direction;
	vec3 t1 = (_max - ray.origin) / ray.direction;

	vec3 t_min = minvec(t0, t1);
	vec3 t_max = maxvec(t0, t1);

	double tmin = ffmax(t_min[0], ffmax(t_min[1], t_min[2]));
	double tmax = ffmin(t_max[0], ffmin(t_max[1], t_max[2]));

	if (tmin > tmax)
		return false;

	return true;
}

AABB surrounding_box(AABB box0, AABB box1) { // calculate bigger AABB with	box1 and box2
	vec3 small = minvec(box0.min_(), box1.min_());
	vec3 big = maxvec(box0.max_(), box1.max_());

	return AABB(small, big);
}

#endif // !AABB_H
