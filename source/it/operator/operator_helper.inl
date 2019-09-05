#ifndef SU_IT_OPERATOR_HELPER_INL
#define SU_IT_OPERATOR_HELPER_INL

#include <cmath>
#include <cstdint>

namespace op {

static inline float round(float x, uint32_t d) noexcept {
    float const f = std::pow(10.f, static_cast<float>(d));
    return static_cast<float>(uint32_t(x * f + 0.5f)) / f;
}

}  // namespace op

#endif
