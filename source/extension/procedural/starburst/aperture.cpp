#include "aperture.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"

namespace procedural { namespace starburst {

Aperture::Aperture(uint32_t num_blades) {
	blades_.resize(num_blades);

	float angle = 0.f;

	for (uint32_t i = 0; i < num_blades; ++i) {
		float delta = (2.f * math::Pi) / static_cast<float>(num_blades);
		float start = static_cast<float>(i) * delta + angle;

		float3 b = float3(std::sin(start), std::cos(start), 0.f);
		float3 a = b + float3(0.f, 0.f, 1.f);
		float3 c = float3(std::sin(start + delta), std::cos(start + delta), 0.f);

		float3 n = math::normalized(math::cross(c - b, a - b));
		float  d = -math::dot(n, a);

		blades_[i] = float3(n.x, n.y, d);
	}
}

float Aperture::evaluate(float2 p, float resolution) {
	float radius = 1.f;

	float d = 1.f;

	for (auto& b : blades_) {
		float t = b.x * p.x + b.y * p.y + radius * b.z;

		if (t < 0.f) {
			return 0.f;
		}

		d = std::min(d, t);
	}

	return std::min(d * 0.5f * resolution, 1.f);
}

}}
