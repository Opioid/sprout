#pragma once

#include "camera.hpp"

namespace scene { namespace camera {

class Stereoscopic : public Camera {

public:

	Stereoscopic(int2 resolution, float ray_max_t);

	virtual ~Stereoscopic();

	void set_interpupillary_distance(float ipd);

protected:

	float3 eye_offsets_[2];
};

}}
