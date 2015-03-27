#pragma once

#include "vector.hpp"

namespace math {

float2 scrambled_hammersley(uint32_t i, uint32_t num_samples, uint32_t r);

float2 ems(uint32_t i, uint32_t num_samples, uint32_t r);

}
