#pragma once

#include "base/math/vector.hpp"
#include "base/flags/flags.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material { namespace bxdf {

enum class Type {
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

struct Result {
	float3 reflection;
	float3 wi;
	float        pdf;

	using Type_flag = flags::Flags<Type>;
	Type_flag type;
};

}}}
