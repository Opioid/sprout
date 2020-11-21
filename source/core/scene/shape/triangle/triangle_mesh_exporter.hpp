#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_EXPORTER_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_EXPORTER_HPP

#include <string>

namespace scene::shape::triangle::serialize {

class Json_handler;

class Exporter {
  public:
    static void write(std::string const& filename, Json_handler& handler);

    static void write_json(std::string const& filename, Json_handler& handler);
};

}  // namespace scene::shape::triangle

#endif
