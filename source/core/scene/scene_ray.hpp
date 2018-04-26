#ifndef SU_CORE_SCENE_RAY_HPP
#define SU_CORE_SCENE_RAY_HPP

#include "base/flags/flags.hpp"
#include "base/math/ray.hpp"

namespace scene {

struct Ray : public math::Ray {
//	enum class Property {
//		Null	= 0
//	};

//	using Properties = flags::Flags<Property>;

	Ray() = default;
	Ray(const float3& origin, const float3& direction,
		float min_t = 0.f, float max_t = 1.f, uint32_t depth = 0,
		float time = 0.f, float wavelength = 0.f);

	float time;
	float wavelength;
//	Properties properties;
};

}

#endif
