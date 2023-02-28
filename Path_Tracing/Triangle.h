#pragma once

#ifndef TRIANGLE_H
#define TRIANGLE_H

#include"Object.h"

class Triangle : public Object {
public:
	Triangle(){};
	Triangle(vec3 p1, vec3 p2, vec3 p3,std::shared_ptr<Material> m,vec3 e = vec3(0)) 
		:p1(p1),p2(p2),p3(p3),material(m)
	{
		normal = unit_vector(cross(p3 - p1,p2 - p1));
	};

	virtual bool hit(const Ray& ray, double t_min, double t_max, Recoord& hit) const override;
	virtual bool hit_aabb(AABB& out_aabb) const override;
private:
	vec3 p1, p2, p3;
	vec3 normal;
	std::shared_ptr<Material> material;
};

bool Triangle::hit(const Ray& ray, double t_min, double t_max, Recoord& hit) const {
	vec3 tempN = dot(ray.direction, normal) < 0 ? normal : -normal;

	double t = (dot(tempN, p1) - dot(ray.origin, tempN)) / dot(ray.direction, tempN);
	if (t < 0.00005 || t < t_min || t > t_max) return false;

	vec3 position = ray.origin + t * ray.direction;
	vec3 c1 = cross(position - p1,p2 - p1);
	vec3 c2 = cross(position - p2,p3 - p2);
	vec3 c3 = cross(position - p3,p1 - p3);
	if (dot(c1, normal) < 0 || dot(c2, normal) < 0 || dot(c3, normal) < 0) return false;

	hit.t = t;
	hit.position = position;
	vec3 outward_notmal = normal;
	hit.set_normal(ray, outward_notmal);
	hit.material = material;

	return true;
}

bool Triangle::hit_aabb(AABB& out_aabb) const {
	vec3 minv = minvec(p1, minvec(p2, p3));
	vec3 maxv = maxvec(p1, maxvec(p2, p3));

	out_aabb = AABB(minv, maxv);

	return true;
}

#endif // !TRIANGLE_H

