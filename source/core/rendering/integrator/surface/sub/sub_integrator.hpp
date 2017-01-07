#pragma once

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector.hpp"

namespace rendering {

class Worker;

namespace integrator { namespace surface { namespace sub {

class Integrator : public integrator::Integrator {

public:

	Integrator(const take::Settings& settings, rnd::Generator& rng);
	virtual ~Integrator();

	virtual float3 li(Worker& worker, const Ray& ray, const Intersection& intersection) = 0;
};

}}}}

