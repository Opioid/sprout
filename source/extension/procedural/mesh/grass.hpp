#pragma once

#include "core/scene/shape/triangle/triangle_mesh_generator.hpp"

namespace procedural { namespace mesh {

class Grass : public scene::shape::triangle::Generator {
public:

	virtual std::shared_ptr<scene::shape::Shape> create_mesh(const rapidjson::Value& mesh_value,
															 scene::shape::triangle::Provider& provider,
															 thread::Pool& thread_pool) final override;
};

}}

