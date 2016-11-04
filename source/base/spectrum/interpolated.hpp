#pragma once

#include <vector>
#include <cstddef>

namespace spectrum {

class Interpolated {

public:

	Interpolated();

	Interpolated(const float* wavelengths, const float* intensities, size_t len);

	float get_start() const;
	float get_end() const;

	float evaluate(float wl) const;

	float integrate(float a, float b) const;

private:

	std::vector<float> wavelengths_;
	std::vector<float> intensities_;
};

}
