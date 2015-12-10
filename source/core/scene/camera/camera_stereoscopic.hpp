#pragma once

#include "camera.hpp"

namespace scene { namespace camera {

class Stereoscopic : public Camera {
public:

	Stereoscopic(float interpupillary_distance,
				 math::uint2 resolution, float ray_max_t, float frame_duration, bool motion_blur);

	virtual ~Stereoscopic();

private:


};

}}
