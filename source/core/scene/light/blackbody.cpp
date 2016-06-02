#include "blackbody.hpp"
#include "base/spectrum/xyz.inl"
#include "base/math/vector.inl"

namespace scene { namespace light {

math::float3 blackbody(float temperature) {
	float arg1 = 1e9f / (temperature * temperature * temperature);
	float arg2 = 1e6f / (temperature * temperature);
	float arg3 = 1e3f / temperature;

	float xc;

	if (temperature >= 1667.f && temperature <= 4000.f) {
		xc = -0.2661239f * arg1 - 0.2343580f * arg2 + 0.8776956f * arg3 + 0.179910f;
	} else if (temperature > 4000.f && temperature <= 25000.f) {
		xc = -3.0258469f * arg1 + 2.1070379f * arg2 + 0.2226347f * arg3 + 0.240390f;
	}

	float xc3 = xc * xc * xc;
	float xc2 = xc * xc;

	float yc;

	if (temperature >= 1667.f && temperature <= 2222.f) {
		yc = -1.1063814f * xc3 - 1.34811020f * xc2 + 2.18555832f * xc - 0.20219683f;
	} else if (temperature > 2222.f && temperature <= 4000.f) {
		yc = -0.9549476f * xc3 - 1.37418593f * xc2 + 2.09137015f * xc - 0.16748867f;
	} else if (temperature > 4000.f && temperature <= 25000.f) {
		yc =  3.0817580f * xc3 - 5.87338670f * xc2 + 3.75112997f * xc - 0.37001483f;
	}

	math::float3 xyz(xc / yc, 1.f, (1.f - xc - yc) / yc);

	return math::normalized(spectrum::xyz_to_linear(xyz));

}

}}
