#pragma once

#ifndef OBJECT_H
#define OBJECT_H

struct Recoord;

#include"Ray.h"
#include"Material.h"
#include"AABB.h"

struct Recoord
{
	double t;
	vec3 position;
	vec3 normal;
	double u;
	double v;
	bool front_face;
	std::shared_ptr<Material> material;

	inline void set_normal(const Ray& ray, const vec3& outward_normal) {
		front_face = dot(ray.direction, outward_normal) < 0;
		normal = front_face ? outward_normal : -outward_normal;
	}
};

class Object {
public:
	virtual bool hit(const Ray& ray, double t_min, double t_max, Recoord& hit) const = 0;
	virtual bool hit_aabb(AABB& out_aabb) const = 0;  // get current object's aabb box
};

#endif // !OBJECT_H
