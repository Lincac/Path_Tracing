#pragma once

#ifndef METALLIC_H
#define METALLIC_H

#include"Material.h"

class Metallic : public Material {
public:
	Metallic(vec3 c,double r = 1.) : color(c), roughness(r) {};

	virtual bool scatter(Ray& ray, const Recoord& hit,double& pdf) const override;

private:
	vec3 reflect(const vec3& v, const vec3& u) const{
		return v - 2 * dot(v, u) * u;
	}
	double roughness;
	vec3 color;
};

bool Metallic::scatter(Ray& ray, const Recoord& hit,double& pdf) const { // 常成员函数只能使用常成员函数
	ray.origin = hit.position;
	ray.direction = reflect(unit_vector(ray.direction), hit.normal) + roughness *  random_unit_vector();

	//return dot(ray.direction, hit.normal) > 0.;
	return true;
}

#endif // !METALLIC_H
