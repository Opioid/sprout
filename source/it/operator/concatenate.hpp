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

namespace it::options {
struct Options;
}

struct Item;

namespace op {

using Pipeline = rendering::postprocessor::Pipeline;

uint32_t concatenate(std::vector<Item> const& items, it::options::Options const& options,
                     Pipeline& pipeline, thread::Pool& threads);
}  // namespace op

#endif
