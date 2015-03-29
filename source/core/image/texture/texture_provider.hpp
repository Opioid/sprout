#pragma once

#include "resource/resource_provider.hpp"

namespace image { namespace texture {

class Texture;

class Provider : public resource::Provider<Texture> {
public:

	virtual std::shared_ptr<Texture> load(const std::string& filename, uint32_t flags = 0);

private:

};

}}

