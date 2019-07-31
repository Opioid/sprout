#ifndef SU_IT_DIFFERENCE_HPP
#define SU_IT_DIFFERENCE_HPP

#include <string>
#include <vector>

namespace resource {
class Manager;
}

struct Item;

uint32_t difference(std::vector<Item> const& item, resource::Manager& manager);

#endif
