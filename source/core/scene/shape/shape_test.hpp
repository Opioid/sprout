#pragma once

namespace scene {

namespace entity { struct Composed_transformation; }

struct Ray;

namespace shape {

struct Intersection;
struct Sample;

namespace testing {

bool check(const Intersection& intersection,
		   const entity::Composed_transformation& transformation,
		   const Ray& ray);

bool check(const Sample& sample);

void test();

}}}
