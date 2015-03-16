#include "sampling.hpp"
#include "vector.inl"
#include "math.hpp"

namespace math {

float2 sample_disk_concentric(math::float2 uv) {
	float sx = 2.f * uv.x - 1.f;
	float sy = 2.f * uv.y - 1.f;

	float r, theta;

	if (sx == 0.f && sy == 0.f) {
		return float2(0.f, 0.f);
	}

	if (sx >= -sy) {
		if (sx > sy) {
			// handle first region of disk
			r = sx;
			if (sy > 0.f) {
				theta = sy / r;
			} else {
				theta = 8.f + sy / r;
			}
		} else {
			// handle second region of disk
			r = sy;
			theta = 2.f - sx / r;
		}
	} else {
		if (sx <= sy) {
			// handle third region of disk
			r = -sx;
			theta = 4.f - sy / r;
		} else {
			// handle fourth region of disk
			r = -sy;
			theta = 6.f + sx / r;
		}
	}

	theta *= pi / 4.f;

	float sin_theta = std::sin(theta);
	float cos_theta = std::cos(theta);

	return float2(cos_theta * r, sin_theta * r);
}

float3 sample_hemisphere_cosine(math::float2 uv) {
	float2 xy = sample_disk_concentric(uv);
	float  z  = std::sqrt(std::max(0.f, 1.f - xy.x * xy.x - xy.y * xy.y));

	return float3(xy.x, xy.y, z);
}

}
