#pragma once

#include "math/vector.hpp"

namespace math {

float scrambled_radical_inverse_vdC(uint32_t bits, uint32_t r);

float2 scrambled_hammersley(uint32_t i, uint32_t num_samples, uint32_t r);

float2 ems(uint32_t i, uint32_t r_0, uint32_t r_1);

}
