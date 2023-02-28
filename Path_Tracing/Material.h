#pragma once

#ifndef MATERIAL_H
#define MATERIAL_H

#include"Ray.h"
#include"Object.h"
#include"Texture.h"

class Material {
public:
	virtual bool scatter(Ray& ray, const Recoord& hit, double& pdf) const { return false; };
	virtual double scatter_pdf(Ray& ray, const Recoord& hit) const { return 0; };
	virtual vec3 emitted(double u,double v) const { return vec3(0); };
	virtual vec3 get_albedo(double u, double v) const { return vec3(0); }
};

#endif // !MATERIAL_H
