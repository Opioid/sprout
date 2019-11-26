#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_GENERATOR_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_GENERATOR_HPP

#include "base/json/json_types.hpp"

namespace resource {
class Manager;

template <typename T>
struct Resource_ptr;
}  // namespace resource

namespace scene::shape {

class Shape;

namespace triangle {

class Provider;

class Generator {
  public:
    virtual ~Generator() {}

    using Shape_ptr = resource::Resource_ptr<Shape>;

    virtual Shape_ptr create_mesh(json::Value const& mesh_value,
                                  resource::Manager& resources) noexcept = 0;
};

}  // namespace triangle
}  // namespace scene::shape

#endif
