#pragma once

#include "resource/resource_provider.hpp"

namespace image {

class Image;

class Provider : public resource::Provider<Image> {
public:

	enum class Flags {
		None = 0,
		Use_as_normal = 1
	};

	virtual std::shared_ptr<Image> load(const std::string& filename, uint32_t flags = 0);

private:

};

}

