#ifndef SU_IT_OPERATOR_ADD_HPP
#define SU_IT_OPERATOR_ADD_HPP

#include <cstdint>
#include <vector>

namespace thread {
class Pool;
}

namespace it::options {
struct Options;
}

struct Item;

using Items = std::vector<Item>;

namespace op {
uint32_t add(Items const& items, it::options::Options const& options, thread::Pool& threads);

uint32_t sub(Items const& items, it::options::Options const& options, thread::Pool& threads);
}  // namespace op

#endif
