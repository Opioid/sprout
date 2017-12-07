#ifndef SU_CORE_SCENE_RAY_HPP
#define SU_CORE_SCENE_RAY_HPP

#include "base/flags/flags.hpp"
#include "base/math/ray.hpp"

namespace scene {

struct Ray : public math::Ray {
	enum class Property {
		Null			= 0,
		Within_volume	= 1 << 0
	};

	using Properties = flags::Flags<Property>;

	Ray() = default;
	Ray(const float3& origin, const float3& direction,
		float min_t = 0.f, float max_t = 1.f,
		float time = 0.f, uint32_t depth = 0,
		Properties properties = Property::Null,
		float ior = 1.f);

	float	 time;
	uint32_t depth;
	Properties properties;
	float    ior;
};

}

#endif
