#pragma once

#include "rendering/integrator/integrator.hpp"

namespace rendering {

class Ao : public Surface_integrator {
public:

	virtual math::float3 li(const Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection) const;
};

class Ao_factory : public Surface_integrator_factory {
public:

	virtual Surface_integrator* create(uint32_t id) const;
};

}
