#pragma once

#ifndef SPHERE_H
#define SPEHRE_H

#include"Object.h"

class Sphere : public Object {
public:
    Sphere(){};
    Sphere(vec3 p,double r,std::shared_ptr<Material> m,vec3 e = vec3(0,0,0)) : position(p), raidus(r) ,material(m) {};

    virtual bool hit(const Ray& ray,double t_min,double t_max,Recoord& hit) const override;
    virtual bool hit_aabb(AABB& out_aabb) const override;

    ~Sphere(){};
private:
    vec3 position;
    double raidus;

    std::shared_ptr<Material> material;

    static void get_sphere_uv(const vec3& p, double& u, double& v) {
        auto theta = acos(-p[1]);
        auto phi = atan2(-p[2], p[0]) + PI;

        u = phi / (2 * PI);
        v = theta / PI;
    }
};

bool Sphere::hit(const Ray& ray, double t_min, double t_max, Recoord& hit) const {
    vec3 oc = ray.origin - position;

    double a = dot(ray.direction, ray.direction);
    double b = 2.0 * dot(oc, ray.direction);
    double c = dot(oc, oc) - raidus * raidus;

    double delta = b * b - 4. * a * c;

    if (delta > 0.) {
        double temp = (-b - sqrt(delta)) / (2.0 * a);
        if (t_min < temp && temp < t_max)
        {
            hit.t = temp;
            hit.position = ray.origin + temp * ray.direction;
            vec3 outward_normal = unit_vector((hit.position - position) / raidus);
            hit.set_normal(ray, outward_normal);
            get_sphere_uv(hit.normal, hit.u, hit.v);
            hit.material = material;
            return true;
        }

        temp = (-b + sqrt(delta)) / (2.0 * a);
        if (t_min < temp && temp < t_max)
        {
            hit.t = temp;
            hit.position = ray.origin + temp * ray.direction;
            vec3 outward_normal = unit_vector((hit.position - position) / raidus);
            hit.set_normal(ray,outward_normal);
            get_sphere_uv(hit.normal, hit.u, hit.v);
            hit.material = material;
            return true;
        }
    }
    return false;
}

bool Sphere::hit_aabb(AABB& out_aabb) const {
    out_aabb = AABB(position - vec3(raidus),position + vec3(raidus));

    return true;
}


#endif // !SPHERE_H
