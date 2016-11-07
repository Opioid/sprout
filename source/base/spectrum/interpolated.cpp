#include "interpolated.hpp"
#include "math/math.hpp"

#include <iostream>

namespace spectrum {

Interpolated::Interpolated() {}

Interpolated::Interpolated(const float* wavelengths, const float* intensities, size_t len) {
	wavelengths_.assign(wavelengths, wavelengths + len);
	intensities_.assign(intensities, intensities + len);
}

float Interpolated::get_start() const {
	if (!wavelengths_.empty()) {
		return wavelengths_[0];
	}

	return 0.f;
}

float Interpolated::get_end() const {
	const size_t len = wavelengths_.size();
	if (len > 0) {
		return wavelengths_[len - 1];
	}

	return 0.f;
}

float Interpolated::evaluate(float wl) const {
	auto result = std::equal_range(wavelengths_.begin(), wavelengths_.end(), wl);

//	size_t index0 = static_cast<size_t>(result.first  - wavelengths_.begin());
//	size_t index1 = static_cast<size_t>(result.second - wavelengths_.begin());

	size_t index = static_cast<size_t>(result.first  - wavelengths_.begin());

	if (result.first == result.second) {
		float wl0 = wavelengths_[index - 1];
		float intensity0 = intensities_[index - 1];
		float wl1 = wavelengths_[index];
		float intensity1 = intensities_[index];
		return math::lerp((wl - wl0) / (wl1 - wl0), intensity0, intensity1);
	} else /*if (index0 + 1 == index1)*/ {
		return intensities_[index];
	}
}

float Interpolated::integrate(float a, float b) const {
	const size_t len = wavelengths_.size();
	if (len < 2) {
		return 0.f;
	}

	float start = std::max(a, get_start());
	float end   = std::min(b, get_end());
	if (end <= start) {
		return 0.f;
	}

	// this integration is only correct for a linearly interpolated function
	// and clamps to zero outside the given range

	auto it = std::lower_bound(wavelengths_.begin(), wavelengths_.end(), start);

	size_t index = std::max(static_cast<size_t>(it - wavelengths_.begin()),
							static_cast<size_t>(1)) - 1;

	float integral = 0.f;
	for (; index + 1 < len && end >= wavelengths_[index]; ++index) {
		float wl0 = wavelengths_[index];
		float wl1 = wavelengths_[index + 1];
		float c0 = std::max(wl0, start);
		float c1 = std::min(wl1, end);

		if (c1 <= c0) {
			continue;
		}

		float i0 = intensities_[index];
		float i1 = intensities_[index + 1];
		float inv = 1.f / (wl1 - wl0);
		float interp0 = math::lerp(i0, i1, (c0 - wl0) * inv);
		float interp1 = math::lerp(i0, i1, (c1 - wl0) * inv);
		integral += 0.5f * (interp0 + interp1) * (c1 - c0);
	}

	return integral;
}

}
