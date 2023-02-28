#pragma once

#ifndef SQUARE_H
#define SQUARE_H

#include"Object.h"

class Square : public Object {
public:
	Square() {};
	Square(vec3 p,vec3 wh,std::shared_ptr<Material> m,vec3 e = vec3(0))
		:position(p),wh(wh),material(m),emissive(e)
	{
	};

	virtual bool hit(const Ray& ray, double t_min, double t_max, Recoord& hit) const override;
	virtual bool hit_aabb(AABB& out_aabb) const override;  
private:
	vec3 position;
	vec3 wh;
	vec3 emissive;
	
	std::shared_ptr<Material> material;
};

bool Square::hit(const Ray& ray, double t_min, double t_max, Recoord& hit) const {
	if (wh[0] == 0) {
		double t = (ray.origin[0] - position[0]) / ray.direction[0];
		if (t < t_min || t > t_max) return false;
		vec3 tempp = ray.origin + t * ray.direction;
		if ((position[1] - wh[1]) < tempp[1] && tempp[1] < (position[1] + wh[1]) && (position[2] - wh[2]) < tempp[2] && tempp[2] < (position[2] + wh[2])) {
			hit.t = t;
			hit.position = tempp;
			hit.normal = dot(ray.direction, vec3(-1, 0, 0)) < 0 ? vec3(1, 0, 0) : vec3(-1, 0, 0);
			hit.albedo = material->get_color();
			hit.material = material;
			hit.emissive = emissive;

			return true;
		}
	}
	else if (wh[1] == 0) {
		double t = (ray.origin[1] - position[1]) / ray.direction[1];
		if (t < t_min || t > t_max) return false;
		vec3 tempp = ray.origin + t * ray.direction;
		if ((position[0] - wh[0]) < tempp[0] && tempp[0] < (position[0] + wh[0]) && (position[2] - wh[2]) < tempp[2] && tempp[2] < (position[2] + wh[2])) {
			hit.t = t;
			hit.position = tempp;
			hit.normal = dot(ray.direction, vec3(0, -1, 0)) < 0 ? vec3(0, 1, 0) : vec3(0, -1, 0);
			hit.albedo = material->get_color();
			hit.material = material;
			hit.emissive = emissive;

			return true;
		}
	}
	else {
		double t = (ray.origin[2] - position[2]) / ray.direction[2];
		if (t < t_min || t > t_max) return false;
		vec3 tempp = ray.origin + t * ray.direction;
		if ((position[0] - wh[0]) < tempp[0] && tempp[0] < (position[0] + wh[0]) && (position[1] - wh[1]) < tempp[1] && tempp[1] < (position[1] + wh[1])) {
			hit.t = t;
			hit.position = tempp;
			hit.normal = dot(ray.direction, vec3(0, 0, -1)) < 0 ? vec3(0, 0, 1) : vec3(0, 0, -1);
			hit.albedo = material->get_color();
			hit.material = material;
			hit.emissive = emissive;

			return true;
		}
	}
	return false;
}

bool Square::hit_aabb(AABB& out_aabb) const {
	vec3 minv = position - wh - vec3(0.00001);
	vec3 maxv = position + wh + vec3(0.00001);

	out_aabb = AABB(minv, maxv);

	return true;
}

#endif // !SQUARE_H
