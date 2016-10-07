#include "texture_provider.hpp"
#include "texture_byte_1_unorm.hpp"
#include "texture_byte_2_unorm.hpp"
#include "texture_byte_2_snorm.hpp"
#include "texture_byte_3_snorm.hpp"
#include "texture_byte_3_srgb.hpp"
#include "texture_byte_3_unorm.hpp"
#include "texture_float_1.hpp"
#include "texture_float_3.hpp"
#include "texture_encoding.hpp"
#include "logging/logging.hpp"
#include "image/image.hpp"
#include "image/image_provider.hpp"
#include "resource/resource_provider.inl"
#include "resource/resource_manager.inl"
#include "base/math/vector.inl"
#include "base/memory/variant_map.inl"

#include "texture_test.hpp"
#include "base/debug/assert.hpp"

namespace image { namespace texture {

Provider::Provider() : resource::Provider<Texture>("Texture") {
	encoding::init();
}

std::shared_ptr<Texture> Provider::load(const std::string& filename,
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
	} else if (Usage::Roughness == usage) {
		channels = Channels::X;
	}

	memory::Variant_map image_options;
	image_options.set("channels", channels);
	image_options.inherit_except(options, "usage");

	try {
		auto image = manager.load<Image>(filename, image_options);
		if (!image) {
			logging::error("Loading texture \"" + filename + "\": Unknown error.");
			return nullptr;
		}

		if (Image::Type::Byte_1 == image->description().type) {
			return std::make_shared<Texture_byte_1_unorm>(image);
		} else if (Image::Type::Byte_2 == image->description().type) {
			if (Usage::Anisotropy == usage) {
				return std::make_shared<Texture_byte_2_snorm>(image);
			} else {
				return std::make_shared<Texture_byte_2_unorm>(image);
			}
		} else if (Image::Type::Byte_3 == image->description().type) {
			if (Usage::Normal == usage) {

				SOFT_ASSERT(testing::is_valid_normal_map(*image.get(), filename));

				return std::make_shared<Texture_byte_3_snorm>(image);
			} else if (Usage::Surface == usage) {
				return std::make_shared<Texture_byte_3_unorm>(image);
			} else {
				return std::make_shared<Texture_byte_3_sRGB>(image);
			}
		} else if (Image::Type::Float_1 == image->description().type) {
			return std::make_shared<Texture_float_1>(image);
		} else if (Image::Type::Float_3 == image->description().type) {
			return std::make_shared<Texture_float_3>(image);
		}
	} catch (const std::exception& e) {
		logging::error("Loading texture \"" + filename + "\": " + e.what() + ".");
	}

	return nullptr;
}

std::shared_ptr<Texture> Provider::load(const void* /*data*/,
										const std::string& /*mount_folder*/,
										const memory::Variant_map& /*options*/,
										resource::Manager& /*manager*/) {
	return nullptr;
}

}}
