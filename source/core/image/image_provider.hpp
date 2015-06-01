#pragma once

#include "resource/resource_provider.hpp"
#include "image/encoding/png/png_reader.hpp"

namespace image {

class Image;

class Provider : public resource::Provider<Image> {
public:

	Provider(thread::Pool& pool);

	enum class Flags {
		None = 0,
		Use_as_normal = 1,
		Use_as_mask   = 2
	};

	virtual std::shared_ptr<Image> load(std::istream& stream, uint32_t flags = 0);

private:

	encoding::png::Reader png_reader_;

};

}

