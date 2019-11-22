#ifndef SU_IT_OPERATOR_CONCATENATE_HPP
#define SU_IT_OPERATOR_CONCATENATE_HPP

#include "base/math/vector2.hpp"

#include <cstdint>
#include <vector>

namespace rendering::postprocessor {
class Pipeline;
}

namespace thread {
class Pool;
}

struct Item;

namespace op {

using Pipeline = rendering::postprocessor::Pipeline;

uint32_t concatenate(std::vector<Item> const& items, uint32_t num_per_row, float2 clip,
                     Pipeline& pipeline, thread::Pool& threads) noexcept;
}  // namespace op

#endif
