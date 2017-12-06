#ifndef SU_CORE_SCENE_RAY_HPP
#define SU_CORE_SCENE_RAY_HPP

#include "base/flags/flags.hpp"
#include "base/math/ray.hpp"

namespace scene {

struct Ray : public math::Ray {
	enum class Properties {
		Null			= 0,
		Visible_volumes	= 1 << 0
	};

	Ray() = default;
	Ray(const float3& origin, const float3& direction,
		float min_t = 0.f, float max_t = 1.f,
		float time = 0.f, uint32_t depth = 0,
		flags::Flags<Properties> properties = Properties::Visible_volumes,
		float ior = 1.f);

	float	 time;
	uint32_t depth;
	flags::Flags<Properties> properties;
	float    ior;
};

}

#endif
