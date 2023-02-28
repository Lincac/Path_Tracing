#pragma once

#ifndef DIELECTRIC_H
#define DIELECTRIC_H


#include"Material.h"

class Dielectric : public Material {
public:
	Dielectric(vec3 c,double r): color(c), rate(r) {};

	virtual bool scatter(Ray& ray, const Recoord& hit,double& pdf) const override;

private:
	double rate;
	vec3 color;

	vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) const;
	double schlick(double cosine, double ref_idx) const;

    vec3 reflect(const vec3& v, const vec3& u) const {
        return v - 2 * dot(v, u) * u;
    }
};

bool Dielectric::scatter(Ray& ray, const Recoord& hit,double& pdf) const {
	double etai_over_etat;
	vec3 outward_normal;
	double cos_theta;
	double sin_theta;
	if (dot(ray.direction, hit.normal) < 0.) {
		outward_normal = hit.normal;
		etai_over_etat = 1. / rate;
		cos_theta = dot(-ray.direction, hit.normal);
		sin_theta = sqrt(1. - cos_theta * cos_theta);
	}
	else {
		outward_normal = -hit.normal;
		etai_over_etat = rate;
		cos_theta = dot(ray.direction, hit.normal);
		sin_theta = sqrt(1. - cos_theta * cos_theta);
	}

	if (etai_over_etat * sin_theta > 1.) {
		ray.origin = hit.position;
		ray.direction = unit_vector(reflect(ray.direction, outward_normal));
		return true;
	}
	else {
		float pro = schlick(cos_theta, etai_over_etat);
		if (random01(engine) < pro) {
			ray.origin = hit.position;
			ray.direction = unit_vector(reflect(ray.direction, outward_normal));
			return true;
		}
		else {
			ray.origin = hit.position;
			ray.direction = unit_vector(refract(ray.direction, outward_normal,etai_over_etat));
			return true;
		}
	}
}

vec3 Dielectric::refract(const vec3& uv, const vec3& n, double etai_over_etat) const{
	auto cos_theta = dot(-uv, n);
	vec3 r_out_parallel = etai_over_etat * (uv + cos_theta * n);
	vec3 r_out_perp = -sqrt(1.0 - r_out_parallel.length_sqrt()) * n;
	return r_out_parallel + r_out_perp;
}

double Dielectric::schlick(double cosine, double ref_idx) const{
	auto r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}

#endif // !DIELECTRIC_H
