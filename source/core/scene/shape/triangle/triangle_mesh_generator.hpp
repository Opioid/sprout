#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_GENERATOR_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_GENERATOR_HPP

#include "base/json/json_types.hpp"

namespace resource {
class Manager;
}

namespace scene::shape {

class Shape;

namespace triangle {

class Provider;

class Generator {
  public:
    using Resources = resource::Manager;

    virtual ~Generator() {}

    virtual uint32_t create_mesh(json::Value const& mesh_value, Resources& resources) = 0;
};

}  // namespace triangle
}  // namespace scene::shape

#endif
