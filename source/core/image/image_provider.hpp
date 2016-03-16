#pragma once

#include "resource/resource_provider.hpp"
#include "image/encoding/png/png_reader.hpp"

namespace image {

class Image;

class Provider : public resource::Provider<Image> {
public:

	Provider();

	virtual std::shared_ptr<Image> load(const std::string& filename,
										const memory::Variant_map& options,
										resource::Manager& manager) final override;

private:

	encoding::png::Reader png_reader_;
};

}

