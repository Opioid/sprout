#pragma once

#include "resource/resource_provider.hpp"
#include "image/encoding/png/png_reader.hpp"

namespace image {

class Image;

class Provider : public resource::Provider<Image> {
public:

	Provider(file::System& file_system);

	virtual std::shared_ptr<Image> load(const std::string& filename, uint32_t flags = 0) final override;

private:

	encoding::png::Reader png_reader_;
};

}

