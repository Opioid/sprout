#pragma once

#include "base/math/vector.hpp"
#include "base/flags/flags.hpp"

namespace sampler {

class Sampler;

}

namespace scene { namespace material {

enum class BxDF_type {
	Reflection				= 1 << 0,
	Transmission			= 1 << 1,
	Diffuse					= 1 << 2,
	Glossy					= 1 << 3,
	Specular				= 1 << 4,
	Diffuse_reflection		= Reflection   | Diffuse,
	Glossy_reflection		= Reflection   | Glossy,
	Specular_reflection		= Reflection   | Specular,
	Diffuse_transmission	= Transmission | Diffuse,
	Glossy_transmission		= Transmission | Glossy,
	Specular_transmission	= Transmission | Specular
};

struct BxDF_result {
	math::float3 reflection;
	math::float3 wi;
	float        pdf;

	typedef flags::Flags<BxDF_type> Type;
	Type type;
};

template<typename Sample>
class BxDF {
public:

	BxDF(const Sample& sample) : sample_(sample) {}

protected:

	const Sample& sample_;
};

}}
