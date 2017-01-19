#pragma once

#include "math/vector.hpp"

namespace thread { class Pool; }

namespace math {

int32_t dft_size(int32_t num);

void dft_1d(float2* result, const float* source, int32_t num);

void idft_1d(float* result, const float2* source, int32_t num);

void dft_2d(float2* result, const float* source, int32_t width, int32_t height);

void dft_2d(float2* result, const float* source, int32_t width, int32_t height, thread::Pool& pool);

}
