#pragma once

#include "resource/resource_provider.hpp"
#include "encoding/png/png_reader.hpp"
#include "procedural/flakes/flakes_provider.hpp"

namespace image {

class Image;

class Provider : public resource::Provider<Image> {

public:

	Provider();
	~Provider();

	virtual std::shared_ptr<Image> load(const std::string& filename,
										const memory::Variant_map& options,
										resource::Manager& manager) final override;

	virtual std::shared_ptr<Image> load(const void* data,
										const std::string& mount_folder,
										const memory::Variant_map& options,
										resource::Manager& manager) final override;

	virtual size_t num_bytes() const final override;

private:

	encoding::png::Reader png_reader_;
	procedural::flakes::Provider flakes_provider_;
};

}

