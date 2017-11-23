#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_EXPORTER_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_EXPORTER_HPP

#include <string>

namespace scene::shape::triangle {

class Json_handler;

class Exporter {

public:

	static void write(const std::string& filename, const Json_handler& handler);
};

}

#endif
