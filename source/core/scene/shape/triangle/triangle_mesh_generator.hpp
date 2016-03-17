#pragma once

#include "base/json/rapidjson_types.hpp"
#include <memory>

namespace thread { class Pool; }

namespace scene { namespace shape {

class Shape;

namespace triangle {

class Provider;

class Generator {
public:

    virtual std::shared_ptr<Shape> create_mesh(const rapidjson::Value& mesh_value,
											   Provider& provider, thread::Pool& thread_pool) = 0;
};

}}}
