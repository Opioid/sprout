#pragma once

#include "rendering/integrator/integrator.hpp"
#include "base/math/vector4.hpp"

namespace scene { namespace volume { class Volume; } }

namespace rendering {

class Worker;

namespace integrator { namespace volume {

class Integrator : public integrator::Integrator {

public:

	using Volume = scene::volume::Volume;

	Integrator(rnd::Generator& rng, const take::Settings& settings);
	virtual ~Integrator();

	virtual float3 transmittance(Worker& worker, const Ray& ray, const Volume& volume) = 0;

	virtual float4 li(Worker& worker, const Ray& ray, const Volume& volume,
					  float3& transmittance) = 0;
};

class Factory {

public:

	Factory(const take::Settings& settings);

	virtual Integrator* create(rnd::Generator& rng) const = 0;

protected:

	const take::Settings& take_settings_;
};

}}}
