#pragma once

namespace scene {

namespace entity { struct Composed_transformation; }

struct Ray;

namespace shape {

struct Intersection;

namespace testing {

bool check(const Intersection& intersection,
		   const entity::Composed_transformation& transformation, const Ray& ray);

void test();

}}}
