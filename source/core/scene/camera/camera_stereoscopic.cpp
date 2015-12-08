#include "camera_stereoscopic.hpp"

namespace scene { namespace camera {

Stereoscopic::Stereoscopic(math::float2 dimensions, rendering::sensor::Sensor* sensor, float ray_max_t,
						   float frame_duration, bool motion_blur) :
	Camera(dimensions, sensor, ray_max_t, frame_duration, motion_blur) {}

Stereoscopic::~Stereoscopic() {}

}}
