#ifndef SU_IT_OPERATOR_DIFFERENCE_HPP
#define SU_IT_OPERATOR_DIFFERENCE_HPP

#include <string>
#include <vector>

namespace thread {
class Pool;
}

struct Item;

namespace op {
uint32_t difference(std::vector<Item> const& item, float clamp, float clip, thread::Pool& pool);
}

#endif
