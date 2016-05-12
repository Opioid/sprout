#pragma once

#include "camera.hpp"

namespace scene { namespace camera {

class Stereoscopic : public Camera {
public:

	Stereoscopic(float interpupillary_distance,
				 math::int2 resolution, float ray_max_t);

	virtual ~Stereoscopic();

protected:

	math::float3 eye_offsets_[2];
};

}}
