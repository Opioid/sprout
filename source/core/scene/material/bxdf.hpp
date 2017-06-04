#pragma once

#include "base/math/vector3.hpp"
#include "base/flags/flags.hpp"

namespace scene { namespace material { namespace bxdf {

enum class Type {
	Reflection				= 1 << 0,
	Transmission			= 1 << 1,
	Diffuse					= 1 << 2,
	Glossy					= 1 << 3,
	Specular				= 1 << 4,
	SSS						= 1 << 5,
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
	float3 h;			// intermediate result, convenient to store here
	float  pdf;
	float  h_dot_wi;	// intermediate result, convenient to store here

	using Type_flag = flags::Flags<Type>;
	Type_flag type;
};

}}}
