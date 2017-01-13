#pragma once

#include "resource/resource_provider.hpp"

namespace image {

class Image;

namespace texture {

class Texture;

class Provider : public resource::Provider<Texture> {

public:

	Provider();

	enum class Usage {
		Unknown,
		Color,
		Normal,
		Anisotropy,
		Roughness,
		Surface,
		Mask
	};

	virtual std::shared_ptr<Texture> load(const std::string& filename,
										  const memory::Variant_map& options,
										  resource::Manager& manager) final override;

	virtual std::shared_ptr<Texture> load(const void* data,
										  const std::string& mount_folder,
										  const memory::Variant_map& options,
										  resource::Manager& manager) final override;

	virtual size_t num_bytes() const final override;
};

}}


