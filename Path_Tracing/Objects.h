#pragma once

#ifndef OBJECTS_H
#define OBJECTS_H

#include"Object.h"
#include<vector>

class Objects : public Object {
public:
	Objects() {};
	Objects(std::shared_ptr<Object> obj) { objs.push_back(obj); }; // add first BVH node
	~Objects() { objs.clear(); };

	virtual bool hit(const Ray& ray, double t_min, double t_max, Recoord& hit) const override;
	virtual bool hit_aabb(AABB& out_aabb) const override;

	void add(std::shared_ptr<Object> obj) { this->objs.push_back(obj); };

	std::vector<std::shared_ptr<Object>> objs;
};

bool Objects::hit(const Ray& ray, double t_min, double t_max, Recoord& hit) const {

	bool hit_anything = false;
	double temp_t = t_max;
	for (auto& obj : objs) {
		if (obj->hit(ray,t_min,temp_t,hit))
		{
			hit_anything = true;
			temp_t = hit.t;
		}
	}

	return hit_anything;
}

bool Objects::hit_aabb(AABB& out_aabb) const {
	//if (objs.empty()) return false;

	//AABB temp_box;
	//bool first_box = true;

	//for (const auto& obj : objs) {
	//	if (!obj->hit_aabb(temp_box, time0, time1)) return false;
	//	out_aabb = first_box ? temp_box : surrounding_box(out_aabb,temp_box);
	//	first_box = false;
	//}

	return true;
}

#endif // !OBJECTS_H
