#pragma once

#ifndef BVH_SAH_H
#define BVH_SAH_H

#include"Objects.h"

class BVH_SAH : public Object {
public:
	BVH_SAH() {};
	BVH_SAH(const Objects& objs,int n)
		: BVH_SAH(objs.objs, 0, objs.objs.size() - 1,n)
	{};

	BVH_SAH(const std::vector<std::shared_ptr<Object>>& list, int start, int end,int n);

	virtual bool hit(const Ray& ray, double t_min, double t_max, Recoord& hit) const override;
	virtual bool hit_aabb(AABB& out_aabb) const override;

private:
	std::shared_ptr<Object> left; // maybe node or object
	std::shared_ptr<Object> right; // maybe node or object

	std::vector<std::shared_ptr<Object>> leafs;
	bool leaf;

	AABB aabb; // current aabb node
}; 

BVH_SAH::BVH_SAH(const std::vector<std::shared_ptr<Object>>& list, int start, int end,int n) {
	auto templist = list; // Const object cannot be referenced recursively 

	if ((end - start + 1) <= n) {
		leafs = std::vector<std::shared_ptr<Object>>((end - start + 1));

		for (int i = 0; i < (end - start + 1); i++)
			leafs[i] = templist[start + i];

		leaf = true;
	}
	else {
		leaf = false;

		double Cost = INF;
		int Axis = 0;
		int Split = (start + end) / 2;
		for (int axis = 0; axis < 3; axis++)
		{
			if (axis == 0) std::sort(templist.begin() + start, templist.begin() + end + 1, [](const std::shared_ptr<Object> a, const std::shared_ptr<Object> b) {
				AABB box_a;
				AABB box_b;
				if (!a->hit_aabb(box_a) || !b->hit_aabb(box_b)) {  // get object's aabb
					std::cerr << "there is no bounding box";
				}
				return box_a.min_()[0] < box_b.min_()[0]; // up sort
				});
			if (axis == 1) std::sort(templist.begin() + start, templist.begin() + end + 1, [](const std::shared_ptr<Object> a, const std::shared_ptr<Object> b) {
				AABB box_a;
				AABB box_b;
				if (!a->hit_aabb(box_a) || !b->hit_aabb(box_b)) {
					std::cerr << "there is no bounding box";
				}
				return box_a.min_()[1] < box_b.min_()[1];
				});
			if (axis == 2) std::sort(templist.begin() + start, templist.begin() + end + 1, [](const std::shared_ptr<Object> a, const std::shared_ptr<Object> b) {
				AABB box_a;
				AABB box_b;
				if (!a->hit_aabb(box_a) || !b->hit_aabb(box_b)) {
					std::cerr << "there is no bounding box";
				}
				return box_a.min_()[2] < box_b.min_()[2];
				});

			//  left Prefixes and  
			std::vector<vec3> leftmin(end - start + 1, vec3(INF));
			std::vector<vec3> leftmax(end - start + 1, vec3(-INF));
			for (int i = start; i <= end; i++)
			{
				int bias = i == start ? 0 : 1;

				AABB temp;
				templist[i]->hit_aabb(temp);
				leftmin[i - start] = minvec(leftmin[i - start - bias], temp.min_());
				leftmax[i - start] = maxvec(leftmax[i - start - bias], temp.max_());
			}

			//  right Prefixes and  
			std::vector<vec3> rightmin(end - start + 1, vec3(INF));
			std::vector<vec3> rightmax(end - start + 1, vec3(-INF));
			for (int i = end; i >= start; i--)
			{
				int bias = i == end ? 0 : 1;

				AABB temp;
				templist[i]->hit_aabb(temp);
				rightmin[i - start] = minvec(rightmin[i - start + bias], temp.min_());
				rightmax[i - start] = maxvec(rightmax[i - start + bias], temp.max_());
			}

			double cost = INF;
			int split = start;
			for (int i = start; i <= end - 1; i++)
			{
				double x, y, z;

				vec3 leftaa = leftmin[i - start];
				vec3 leftbb = leftmax[i - start];
				x = leftbb[0] - leftaa[0];
				y = leftbb[1] - leftaa[1];
				z = leftbb[2] - leftaa[2];
				double lefts = 2.0 * (x * y + x * z + y * z);
				double leftcost = lefts * (i - start + 1);

				vec3 rightaa = rightmin[i - start + 1];
				vec3 rightbb = rightmax[i - start + 1];
				x = rightbb[0] - rightaa[0];
				y = rightbb[1] - rightaa[1];
				z = rightbb[2] - rightaa[2];
				double rights = 2.0 * (x * y + x * z + y * z);
				double rightcost = rights * (end - start);

				double totalcost = leftcost + rightcost;
				if (totalcost < cost) {
					cost = totalcost;
					split = i;
				}
			}

			if (cost < Cost) {
				Cost = cost;
				Split = split;
				Axis = axis;
			}
		}

		if (Axis == 0) std::sort(templist.begin() + start, templist.begin() + end + 1, [](const std::shared_ptr<Object> a, const std::shared_ptr<Object> b) {
			AABB box_a;
			AABB box_b;
			if (!a->hit_aabb(box_a) || !b->hit_aabb(box_b)) {  // get object's aabb
				std::cerr << "there is no bounding box";
			}
			return box_a.min_()[0] < box_b.min_()[0]; // up sort
			});
		if (Axis == 1) std::sort(templist.begin() + start, templist.begin() + end + 1, [](const std::shared_ptr<Object> a, const std::shared_ptr<Object> b) {
			AABB box_a;
			AABB box_b;
			if (!a->hit_aabb(box_a) || !b->hit_aabb(box_b)) {
				std::cerr << "there is no bounding box";
			}
			return box_a.min_()[1] < box_b.min_()[1];
			});
		if (Axis == 2) std::sort(templist.begin() + start, templist.begin() + end + 1, [](const std::shared_ptr<Object> a, const std::shared_ptr<Object> b) {
			AABB box_a;
			AABB box_b;
			if (!a->hit_aabb(box_a) || !b->hit_aabb(box_b)) {
				std::cerr << "there is no bounding box";
			}
			return box_a.min_()[2] < box_b.min_()[2];
			});

		left = std::make_shared<BVH_SAH>(templist, start, Split, n);
		right = std::make_shared<BVH_SAH>(templist, Split + 1, end, n);
	}

	vec3 boxmin = vec3(-INF);
	vec3 boxmax = vec3(INF);

	for (int i = start; i <= end; i++)
	{
		AABB temp;
		templist[i]->hit_aabb(temp);
		boxmin = minvec(boxmin, temp.min_());
		boxmax = maxvec(boxmax, temp.max_());
	}

	aabb = AABB(boxmin, boxmax);
};

bool BVH_SAH::hit(const Ray& ray, double t_min, double t_max, Recoord& hit) const {
	if (!aabb.hit_aabb(ray)) return false;

	if (leaf) {
		bool hit_anything = false;
		double hit_max = t_max;
		for (auto& temp : leafs)
		{
			if (temp->hit(ray, t_min, hit_max, hit)) {
				hit_anything = true;
				hit_max = hit.t;
			}
		}
		return hit_anything;
	}
	else {
	}

	bool leftnode = left->hit(ray, t_min, t_max, hit);
	bool rightnode = right->hit(ray, t_min, leftnode ? hit.t : t_max, hit);

	return leftnode || rightnode;
}

bool BVH_SAH::hit_aabb(AABB& out_aabb) const {
	out_aabb = aabb;

	return true;
}

#endif // !BVH_SAH
