#pragma once

#include "triangle_mesh_provider.hpp"
#include "base/json/rapidjson_types.hpp"

namespace scene { namespace shape { namespace triangle {

class Generator {
public:

    virtual std::shared_ptr<Shape> create_mesh(const rapidjson::Value& mesh_value,
                                               Provider provider, thread::Pool& thread_pool) = 0;
};

}}}
