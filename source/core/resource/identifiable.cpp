#include "identifiable.hpp"
#include <vector>

namespace resource {

static std::vector<std::string> type_names;

uint32_t id(std::string const& type_name) {
    uint32_t const len = static_cast<uint32_t>(type_names.size());

    for (uint32_t i = 0; i < len; ++i) {
        if (type_names[i] == type_name) {
            return i;
        }
    }

    type_names.push_back(type_name);

    return len;
}

std::string_view name(uint32_t id) {
    return type_names[id];
}

}  // namespace resource
