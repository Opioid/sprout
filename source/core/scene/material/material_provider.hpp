#pragma once

#include "resource/resource_provider.hpp"

namespace rendering { namespace material {

class Material;

}}

namespace scene { namespace material {

class Provider : public resource::Provider<rendering::material::Material> {
public:

	virtual std::shared_ptr<rendering::material::Material> load(const std::string& filename) const;

private:

};

}}
