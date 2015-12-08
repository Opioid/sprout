#pragma once

#include "camera.hpp"

namespace scene { namespace camera {

class Stereoscopic : public Camera {
public:

	Stereoscopic(float interpupillary_distance,
				 math::float2 dimensions, rendering::sensor::Sensor* sensor, float ray_max_t,
				 float frame_duration, bool motion_blur);

	virtual ~Stereoscopic();

private:


};

}}
