#include "camera_stereoscopic.hpp"

namespace scene { namespace camera {

Stereoscopic::Stereoscopic(float interpupillary_distance,
						   math::uint2 resolution, float ray_max_t, float frame_duration, bool motion_blur) :
	Camera(resolution, ray_max_t, frame_duration, motion_blur) {}

Stereoscopic::~Stereoscopic() {}

}}
