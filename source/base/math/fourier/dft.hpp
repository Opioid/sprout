#pragma once

#include "math/vector.hpp"

namespace math {

size_t dft_size(size_t num);

void dft_1d(float2* result, const float* source, size_t num);

void idft_1d(float* result, const float2* source, size_t num);

void dft_2d(float2* result, const float* source, size_t width, size_t height);

void idft_2d(float* result, const float2* source, size_t width, size_t height);

}
