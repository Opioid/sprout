#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_GENERATOR_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_GENERATOR_HPP

#include <memory>
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
    virtual std::shared_ptr<Shape> create_mesh(json::Value const& mesh_value,
                                               resource::Manager& manager) = 0;
};

}  // namespace triangle
}  // namespace scene::shape

#endif
