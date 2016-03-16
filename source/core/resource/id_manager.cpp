#include "id_manager.hpp"

namespace resource {

ID_manager id_manager;

uint32_t ID_manager::id(const std::string& type_name) {
	for (uint32_t i = 0, len = static_cast<uint32_t>(get_names().size()); i < len; ++i) {
		if (get_names()[i] == type_name) {
			return i;
		}
	}

	get_names().push_back(type_name);

	return static_cast<uint32_t>(get_names().size() - 1);
}

std::string ID_manager::name(uint32_t id) const {
	return get_names()[id];
}

std::vector<std::string>& ID_manager::get_names() const {
	static std::vector<std::string> type_names;
	return type_names;
}

}
