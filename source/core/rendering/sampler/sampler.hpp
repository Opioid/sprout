#pragma once

#include "base/math/vector.hpp"

namespace sampler {

struct Camera_sample;

class Sampler {
public:

	virtual bool generate_camera_sample(const math::float2& offset, Camera_sample& sample) = 0;
};

}
