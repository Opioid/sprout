#pragma once

#include "resource/resource_provider.hpp"

namespace image {

class Image;

class Provider : public resource::Provider<Image> {
public:

	virtual std::shared_ptr<Image> load(const std::string& filename, uint32_t flags = 0);

private:

};

}

