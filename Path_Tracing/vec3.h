#pragma once

#include"Global.h"

#ifndef VEC3_H
#define VEC3_H

static const double PI = 3.14159265357;

static const double INF = 21474836.0;

class vec3
{
public:
	vec3()  : v{ 0,0,0 } {};
	vec3(double e) : v{e,e,e} {};
	vec3(double e0, double e1, double e2) : v{e0,e1,e2} {};

	vec3 operator-()	const {
		return vec3(-v[0], -v[1], -v[2]);
	};

	double operator[](int i) const {
		return v[i];
	}

	double& operator[](int i) {
		return v[i];
	}

	vec3& operator+= (const vec3& e) {
		v[0] += e[0];
		v[1] += e[1];
		v[2] += e[2];
		
		return *this;
	}

	vec3& operator*= (const double t) {
		v[0] *= t;
		v[1] *= t;
		v[2] *= t;

		return *this;
	}
	vec3& operator*= (const vec3& u) {
		v[0] *= u[0];
		v[1] *= u[1];
		v[2] *= u[2];

		return *this;
	}

	vec3& operator/=(const double t) {
		v[0] /= t;
		v[1] /= t;
		v[2] /= t;

		return *this;
	}

	double length() const {
		return sqrt(length_sqrt());
	}

	double length_sqrt() const {
		return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	}

private:
	double v[3];

};

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
	return out << v[0] << v[1] << v[2];
}

inline vec3 operator+(const vec3& v, const vec3& u) { // 重载运算符为非成员函数，需要两个参数
	return vec3(v[0] + u[0], v[1] + u[1], v[2] + u[2]);
}

inline vec3 operator-(const vec3& v, const vec3& u) {
	return vec3(v[0] - u[0], v[1] - u[1], v[2] - u[2]);
}

inline vec3 operator*(const vec3& v, const vec3& u) {
	return vec3(v[0] * u[0], v[1] * u[1], v[2] * u[2]);
}

inline vec3 operator*(const vec3& v, double t) {
	return vec3(v[0] * t, v[1] * t, v[2] * t);
}

inline vec3 operator*(double t,const vec3& v) {
	return vec3(v[0] * t, v[1] * t, v[2] * t);
}

inline vec3 operator/(const vec3& v, double t) {
	return v * (1 / t);
}

inline vec3 operator/(const vec3& v, const vec3& u) {
	vec3 temp;
	temp[0] = v[0] / u[0];
	temp[1] = v[1] / u[1];
	temp[2] = v[2] / u[2];
	return temp;
}

inline double dot(const vec3& v, const vec3& u) {
	return v[0] * u[0] + v[1] * u[1] + v[2] * u[2];
}

inline vec3 cross(const vec3& v, const vec3& u) {  // here is left hand rule
	return vec3(u[1] * v[2] - u[2] * v[1],
		u[2] * v[0] - u[0] * v[2],
		u[0] * v[1] - u[1] * v[0]);
}

inline vec3 unit_vector(vec3 v) {
	return v / v.length();
}

inline vec3 random_unit_vector() {
	auto a = random01(engine) * 2. * PI;
	auto z = random01(engine) * 2. - 1.;
	auto r = sqrt(1 - z * z);
	return vec3(r * cos(a), r * sin(a), z);
}

inline vec3 minvec(vec3 t1, vec3 t2) {
	vec3 temp;
	temp[0] = ffmin(t1[0], t2[0]);
	temp[1] = ffmin(t1[1], t2[1]);
	temp[2] = ffmin(t1[2], t2[2]);

	return temp;
}

inline vec3 maxvec(vec3 t1, vec3 t2) {
	vec3 temp;
	temp[0] = ffmax(t1[0], t2[0]);
	temp[1] = ffmax(t1[1], t2[1]);
	temp[2] = ffmax(t1[2], t2[2]);

	return temp;
}

#endif // !VEC3_H
