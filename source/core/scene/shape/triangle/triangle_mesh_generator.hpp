#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_GENERATOR_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_GENERATOR_HPP

#include "base/json/json_types.hpp"
#include <memory>

namespace resource { class Manager; }

namespace scene::shape {

class Shape;

namespace triangle {

class Provider;

class Generator {

public:

	virtual std::shared_ptr<Shape> create_mesh(const json::Value& mesh_value,
											   resource::Manager& manager) = 0;
};

}}

#endif
