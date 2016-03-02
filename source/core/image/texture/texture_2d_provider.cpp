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
#include "resource/resource_cache.inl"
#include "base/math/vector.inl"
#include "base/memory/variant_map.inl"

namespace image { namespace texture {

Provider::Provider(file::System& file_system, thread::Pool& thread_pool) :
	resource::Provider<Texture_2D>(file_system, thread_pool),
	image_provider_(file_system_, thread_pool),
	image_cache_(image_provider_) {
	encoding::init();
}

std::shared_ptr<Texture_2D> Provider::load(const std::string& filename, const memory::Variant_map& options) {
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

	bool was_cached;

	auto image = image_cache_.load(filename, image_options, was_cached);
	if (!image) {
		return nullptr;
	}

	int32_t num_elements = 1;
	options.query("num_elements", num_elements);
	if (num_elements > 1) {
		image->set_num_elements(num_elements);
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
