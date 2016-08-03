#include "camera_stereoscopic.hpp"
#include "base/math/vector.inl"

namespace scene { namespace camera {

Stereoscopic::Stereoscopic(int2 resolution, float ray_max_t) :
	Camera(resolution, ray_max_t) {

}

Stereoscopic::~Stereoscopic() {}

void Stereoscopic::set_interpupillary_distance(float ipd) {
	float half_ipd = 0.5f * ipd;
	eye_offsets_[0] = float3(-half_ipd, 0.f, 0.f);
	eye_offsets_[1] = float3( half_ipd, 0.f, 0.f);
}

}}
