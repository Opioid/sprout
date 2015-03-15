#pragma once

#include "surrounding.hpp"

namespace scene { namespace surrounding {

class Uniform : public Surrounding {
public:

	Uniform(const math::float3& energy);

	virtual math::float3 sample(const math::Oray& ray) const;

private:

	math::float3 energy_;
};

}}
