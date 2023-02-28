#pragma once

#ifndef LIGHT_H
#define LIGHT_H

#include"Material.h"

class Light : public Material {
public:
	Light() {};
	Light(std::shared_ptr<Texture> E):emit(E) {};

	virtual vec3 emitted(double u,double v) const override { 
		return  emit->value(u,v);
	};

private:
	std::shared_ptr<Texture> emit;
};
#endif // !LIGHT_H

