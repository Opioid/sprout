#include "surrounding_uniform.hpp"

namespace scene { namespace surrounding {

Uniform::Uniform(const math::float3& energy) : energy_(energy) {}

math::float3 Uniform::sample(const math::Oray& ray) const {
	return energy_;
}

}}
