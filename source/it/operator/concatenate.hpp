#ifndef SU_IT_OPERATOR_CONCATENATE_HPP
#define SU_IT_OPERATOR_CONCATENATE_HPP

#include <cstdint>
#include <vector>

namespace thread {
class Pool;
}

struct Item;

namespace op {
uint32_t concatenate(std::vector<Item> const& items, uint32_t num_per_row,
                     thread::Pool& pool) noexcept;
}

#endif
