#pragma once

#include <string>
#include <memory>

namespace take {

struct Take;

class Loader {
public:

	std::shared_ptr<Take> load(const std::string& filename);
};

}
