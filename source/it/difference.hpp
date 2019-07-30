#ifndef SU_IT_DIFFERENCE_HPP
#define SU_IT_DIFFERENCE_HPP

#include <string>
#include <vector>

namespace resource {
class Manager;
}

uint32_t difference(std::vector<std::string> const& images, resource::Manager& manager);

#endif
