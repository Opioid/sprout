#pragma once

#include <vector>
#include <string>

namespace resource {

class ID_manager {

public:

	uint32_t id(std::string const& type_name);

	std::string const& name(uint32_t id) const;

private:

	std::vector<std::string>& names() const;
};

extern ID_manager id_manager;

}
