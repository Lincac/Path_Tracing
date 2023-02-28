#pragma once

#ifndef PLANE_H
#define PLANE_H

#include"Object.h"

class Plane : public Object {
public:
	Plane() {};
	Plane(vec3 p,std::shared_ptr<Material> m,vec3 e = vec3(0,0,0)) : position(p) ,material(m),emissive(e) {};

	virtual bool hit(const Ray& ray, double t_min, double t_max, Recoord& hit) const override;
	virtual bool hit_aabb(AABB& out_aabb) const override;

	~Plane(){};
private:
	vec3 position;
	vec3 emissive;
	std::shared_ptr<Material> material;

};

bool Plane::hit(const Ray& ray, double t_min, double t_max, Recoord& hit) const {
	double temp = (position[1] - ray.origin[1]) / ray.direction[1];
	if (temp > 0.) {
		if (t_min < temp && temp < t_max) {
			hit.t = temp;
			hit.position = ray.origin + temp * ray.direction;
			hit.normal = vec3(0, 1, 0);
			hit.material = material;

			return true;
		}
	}
	return false;
}

bool Plane::hit_aabb(AABB& out_aabb) const {
	out_aabb = AABB(position - vec3(0,0.0001,0), position + vec3(0, 0, 0));
	return true;
}


#endif // !PLANE_H
