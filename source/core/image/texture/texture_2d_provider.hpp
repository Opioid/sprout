#pragma once

#include "resource/resource_provider.hpp"

namespace image {

class Image;

namespace texture {

class Texture_2D;

class Provider : public resource::Provider<Texture_2D> {

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

	virtual std::shared_ptr<Texture_2D> load(const std::string& filename,
											 const memory::Variant_map& options,
											 resource::Manager& manager) final override;
};

}}


