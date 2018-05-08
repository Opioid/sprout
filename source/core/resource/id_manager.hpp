#ifndef SU_CORE_RESOURCE_ID_MANAGER_HPP
#define SU_CORE_RESOURCE_ID_MANAGER_HPP

#include <vector>
#include <string>

namespace resource {

class ID_manager {

public:

	uint32_t id(std::string const& type_name);

	std::string_view name(uint32_t id) const;

private:

	std::vector<std::string>& names() const;
};

extern ID_manager id_manager;

}

#endif
