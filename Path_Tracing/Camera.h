#pragma once

#ifndef  CAMERA_H
#define CAMERA_H

#include"Ray.h"

class Camera {
public:
	Camera(vec3 lookform,vec3 lookat, vec3 up, double fov,double aspect) {

		auto theta = fov / PI;
		auto half_height = tan(theta / 2);
		auto half_width = aspect * half_height;

		vec3 w = unit_vector(lookat - lookform);
		vec3 u = unit_vector(cross(w,up));
		vec3 v = cross(u, w);

		lower_left_corner = origin - half_width * u - half_height * v - w;
		horizontal = 2 * half_width * u;
		vertical = 2 * half_height * v;
		origin = lookform;
	}

	Ray get_ray(double u, double v) {
		return Ray(origin,unit_vector(lower_left_corner + u * horizontal + v * vertical - origin));
	}

private:
	vec3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
	vec3 origin;
};

#endif // ! CAMERA_H
