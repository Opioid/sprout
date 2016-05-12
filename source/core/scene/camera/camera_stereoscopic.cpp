#include "camera_stereoscopic.hpp"
#include "base/math/vector.inl"

namespace scene { namespace camera {

Stereoscopic::Stereoscopic(float interpupillary_distance,
						   math::int2 resolution, float ray_max_t) :
	Camera(resolution, ray_max_t) {
	float half_ipd = 0.5f * interpupillary_distance;
	eye_offsets_[0] = math::float3(-half_ipd, 0.f, 0.f);
	eye_offsets_[1] = math::float3( half_ipd, 0.f, 0.f);
}

Stereoscopic::~Stereoscopic() {}

}}
