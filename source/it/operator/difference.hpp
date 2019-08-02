#ifndef SU_IT_OPERATOR_DIFFERENCE_HPP
#define SU_IT_OPERATOR_DIFFERENCE_HPP

#include <cstdint>
#include <vector>
#include "base/math/vector2.hpp"

namespace thread {
class Pool;
}

struct Item;

namespace op {
uint32_t difference(std::vector<Item> const& items, float clamp, float2 clip,
                    thread::Pool& pool) noexcept;
}

#endif
