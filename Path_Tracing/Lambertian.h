#pragma once

#ifndef LAMBERTIAN_H
#define LAMBERTIAN_H

#include"Material.h"

class Lambertian : public Material {
public:
	Lambertian(std::shared_ptr<Texture> c) :color(c) {};

	virtual bool scatter(Ray& ray, const Recoord& hit,double& pdf) const override;

	virtual double scatter_pdf(Ray& ray, const Recoord& hit) const override{
		auto cosine = dot(hit.normal, ray.direction);
		return cosine < 0 ? 0 : cosine / PI;
	};

	virtual vec3 get_albedo(double u, double v) const override { return color->value(u, v); };

private:
	std::shared_ptr<Texture> color;
};

bool Lambertian::scatter(Ray& ray, const Recoord& hit,double& pdf) const {
	ray.origin = hit.position;
	ray.direction = unit_vector(hit.normal + random_unit_vector());
	
	pdf = dot(hit.normal, ray.direction) / PI;

	return true;
}
#endif // !LAMBERTIAN_H
