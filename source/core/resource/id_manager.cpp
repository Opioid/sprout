#include "id_manager.hpp"

namespace resource {

ID_manager id_manager;

uint32_t ID_manager::id(std::string const& type_name) {
    auto& n = names();

    for (uint32_t i = 0, len = static_cast<uint32_t>(n.size()); i < len; ++i) {
        if (n[i] == type_name) {
            return i;
        }
    }

    n.push_back(type_name);

    return static_cast<uint32_t>(n.size() - 1);
}

std::string_view ID_manager::name(uint32_t id) const {
    return names()[id];
}

std::vector<std::string>& ID_manager::names() const {
    static std::vector<std::string> type_names;
    return type_names;
}

}  // namespace resource
