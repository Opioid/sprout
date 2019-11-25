#ifndef SU_IT_OPERATOR_STATISTICS_HPP
#define SU_IT_OPERATOR_STATISTICS_HPP

#include <vector>

namespace thread {
class Pool;
}

namespace it::options {
struct Options;
}

struct Item;

namespace op {
uint32_t statistics(std::vector<Item> const& items, it::options::Options const& options,
                    thread::Pool& threads) noexcept;
}

#endif
