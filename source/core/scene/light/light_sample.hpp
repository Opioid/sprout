#ifndef SU_CORE_SCENE_LIGHT_SAMPLE_HPP
#define SU_CORE_SCENE_LIGHT_SAMPLE_HPP

#include "scene/shape/shape_sample.hpp"
#include "base/math/vector3.hpp"

namespace scene::light {

struct Sample {
	shape::Sample shape;
	float3		  radiance;
};

}

#endif
