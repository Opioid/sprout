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
		Undefined,
		Color,
		Normal,
		Anisotropy,
		Roughness,
		Surface,
		Specularity,
		Mask
	};

	virtual std::shared_ptr<Texture> load(const std::string& filename,
										  const memory::Variant_map& options,
										  resource::Manager& manager) override final;

	virtual std::shared_ptr<Texture> load(const void* data,
										  const std::string& mount_folder,
										  const memory::Variant_map& options,
										  resource::Manager& manager) override final;

	virtual size_t num_bytes() const override final;
};

}}


