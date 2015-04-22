#include "ggx.hpp"
#include "base/math/math.hpp"
#include <cmath>

namespace scene { namespace material { namespace ggx {

inline math::float3 f(float wo_dot_h, const math::float3& f0) {
	return f0 + (std::exp2((-5.55473f * wo_dot_h - 6.98316f) * wo_dot_h)) * math::float3(1.f - f0.x, 1.f - f0.y, 1.f - f0.z);
}

inline float d(float n_dot_h, float a2) {
	a2 = std::max(a2, 0.00000001f);
	float d = n_dot_h * n_dot_h * (a2 - 1.f) + 1.f;
	return a2 / (math::Pi * d * d);
}

inline float g(float n_dot_wi, float n_dot_wo, float a2) {
	float gwo = n_dot_wo + std::sqrt((n_dot_wo - n_dot_wo * a2) * n_dot_wo + a2);
	float gwi = n_dot_wi + std::sqrt((n_dot_wi - n_dot_wi * a2) * n_dot_wi + a2);
	return 1.f / std::sqrt(gwo * gwi);
}

}}}

