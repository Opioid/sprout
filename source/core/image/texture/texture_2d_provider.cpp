#include "texture_2d_provider.hpp"
#include "texture_2d_byte_1_unorm.hpp"
#include "texture_2d_byte_2_unorm.hpp"
#include "texture_2d_byte_2_snorm.hpp"
#include "texture_2d_byte_3_snorm.hpp"
#include "texture_2d_byte_3_srgb.hpp"
#include "texture_2d_byte_3_unorm.hpp"
#include "texture_2d_float_3.hpp"
#include "texture_encoding.hpp"
#include "image/image.hpp"
#include "image/image_provider.hpp"
#include "resource/resource_provider.inl"
#include "resource/resource_manager.inl"
#include "resource/resource_cache.inl"
#include "base/math/vector.inl"
#include "base/memory/variant_map.inl"

namespace image { namespace texture {

Provider::Provider() : resource::Provider<Texture_2D>("Texture_2D") {
	encoding::init();
}

std::shared_ptr<Texture_2D> Provider::load(const std::string& filename,
										   const memory::Variant_map& options,
										   resource::Manager& manager) {
	Channels channels = Channels::XYZ;

	Usage usage = Usage::Unknown;
	options.query("usage", usage);

	if (Usage::Mask == usage) {
		channels = Channels::W;
	} else if (Usage::Anisotropy == usage) {
		channels = Channels::XY;
	} else if (Usage::Surface == usage) {
		channels = Channels::XY;
	}

	memory::Variant_map image_options;
	image_options.insert("channels", channels);
	image_options.inherit("num_elements", options);

	auto image = manager.load<Image>(filename, image_options);
	if (!image) {
		return nullptr;
	}

	if (Image::Type::Byte_1 == image->description().type) {
		return std::make_shared<Texture_2D_byte_1_unorm>(image);
	} else if (Image::Type::Byte_2 == image->description().type) {
		if (Usage::Anisotropy == usage) {
			return std::make_shared<Texture_2D_byte_2_snorm>(image);
		} else {
			return std::make_shared<Texture_2D_byte_2_unorm>(image);
		}
	} else if (Image::Type::Byte_3 == image->description().type) {
		if (Usage::Normal == usage) {
			return std::make_shared<Texture_2D_byte_3_snorm>(image);
		} else if (Usage::Surface == usage) {
			return std::make_shared<Texture_2D_byte_3_unorm>(image);
		} else {
			return std::make_shared<Texture_2D_byte_3_sRGB>(image);
		}
	} else if (Image::Type::Float_3 == image->description().type) {
		return std::make_shared<Texture_2D_float_3>(image);
	}

	return nullptr;
}

}}
