#pragma once

#include "resource/resource_provider.hpp"
#include "scene/shape/geometry/vertex.hpp"
#include "base/json/rapidjson_types.hpp"
#include <string>
#include <memory>
#include <vector>

namespace scene { namespace shape {

struct Vertex;

namespace triangle {

struct Index_triangle;
class Mesh;

class Provider : public resource::Provider<Mesh> {
public:

	Provider(file::System& file_system);

	virtual std::shared_ptr<Mesh> load(const std::string& filename, uint32_t flags = 0);
};

}}}
