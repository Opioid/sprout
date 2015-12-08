#pragma once

#include "camera.hpp"

namespace scene { namespace camera {

class Stereoscopic : public Camera {
public:

	Stereoscopic(math::float2 dimensions, rendering::sensor::Sensor* sensor, float ray_max_t,
				 float frame_duration, bool motion_blur);

	virtual ~Stereoscopic();
};

}}
