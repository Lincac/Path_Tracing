#pragma once

#ifndef BVH_H
#define BVH_H

#include"Objects.h"

class BVH : public Object{
public:
	BVH() {};
	BVH(const Objects& objs) 
		: BVH(objs.objs,0,objs.objs.size())
	{
	};

	BVH(const std::vector<std::shared_ptr<Object>>& list, size_t start, size_t end);

	virtual bool hit(const Ray& ray, double t_min, double t_max, Recoord& hit) const override;
	virtual bool hit_aabb(AABB& out_aabb) const override;

private:
	std::shared_ptr<Object> left; // maybe node or object
	std::shared_ptr<Object> right; // maybe node or object

	AABB aabb; // current aabb node
};

inline bool box_compare(const std::shared_ptr<Object> a, const std::shared_ptr<Object> b,int axis) {
	AABB box_a;
	AABB box_b;

	if (!a->hit_aabb(box_a) || !b->hit_aabb(box_b)) {  // get object's aabb
		std::cerr << "there is no bounding box";
	}

	return box_a.min_()[axis] < box_b.min_()[axis]; // up sort
}

bool box_x_compare(const std::shared_ptr<Object> a, const std::shared_ptr<Object> b) {
	return box_compare(a, b, 0);
}

bool box_y_compare(const std::shared_ptr<Object> a, const std::shared_ptr<Object> b) {
	return box_compare(a, b, 1);
}

bool box_z_compare(const std::shared_ptr<Object> a, const std::shared_ptr<Object> b) {
	return box_compare(a, b, 2);
}
BVH::BVH(const std::vector<std::shared_ptr<Object>>& list, size_t start, size_t end) {
	
	auto objects = list;


	int axis = random01(engine) * 3;
	auto compare = (axis == 0) ? box_x_compare : (axis == 1) ? box_y_compare : box_z_compare;

		
	size_t obj_span = end - start;
	if (obj_span == 1) {
		left = right = objects[start];
	}
	else if (obj_span == 2) {
		if (compare(objects[start], objects[start + 1])) {
			left = objects[start];
			right = objects[start + 1];
		}
		else {
			left = objects[start + 1];
			right = objects[start];
		}
	}
	else
	{
		std::sort(objects.begin() + start, objects.begin() + end, compare);

		auto mid = start + obj_span / 2;

		left = std::make_shared<BVH>(objects, start, mid);
		right = std::make_shared<BVH>(objects, mid, end);
	}

	AABB box_left, box_right;
	if (!left->hit_aabb(box_left) || !right->hit_aabb(box_right)) {
		std::cerr << "there is no bouding box";
	}

	aabb = surrounding_box(box_left, box_right);
};

bool BVH::hit(const Ray& ray, double t_min, double t_max, Recoord& hit) const {
	if (!aabb.hit_aabb(ray)) return false; 

	bool left_hit = left->hit(ray, t_min, t_max, hit);	 // if node search down until it's object
	bool right_hit = right->hit(ray, t_min, left_hit ? hit.t : t_max, hit); // if node search down until it's object

	return left_hit || right_hit;
}

bool BVH::hit_aabb(AABB& out_aabb) const {
	out_aabb = aabb;

	return true;
}

#endif // !BVH_H
