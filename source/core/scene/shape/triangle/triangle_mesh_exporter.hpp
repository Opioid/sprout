#pragma once

#include <string>

namespace scene { namespace shape { namespace triangle {

class Json_handler;

class Exporter {

public:

	static void write(const std::string& filename, const Json_handler& handler);
};

}}}
