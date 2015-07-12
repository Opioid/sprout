#include "texture_2d_provider.hpp"
#include "texture_2d_byte_1_unorm.hpp"
#include "texture_2d_byte_2_unorm.hpp"
#include "texture_2d_byte_3_snorm.hpp"
#include "texture_2d_byte_3_srgb.hpp"
#include "texture_2d_byte_3_unorm.hpp"
#include "texture_2d_float_3.hpp"
#include "texture_encoding.hpp"
#include "image/image.hpp"
#include "image/image_provider.hpp"
#include "resource/resource_provider.inl"
#include "resource/resource_cache.inl"

namespace image { namespace texture {

Provider::Provider(file::System& file_system) :
	resource::Provider<Texture_2D>(file_system),
	image_provider_(file_system_),
	image_cache_(image_provider_) {
	encoding::init();
}

std::shared_ptr<Texture_2D> Provider::load(const std::string& filename, uint32_t flags) {
	auto stream_pointer = file_system_.read_stream(filename);
	if (!*stream_pointer) {
		throw std::runtime_error("File \"" + filename + "\" could not be opened");
	}

	uint32_t num_channels = 3;

	if (static_cast<uint32_t>(Provider::Flags::Use_as_mask) == flags) {
		num_channels = 1;
	} else if (static_cast<uint32_t>(Provider::Flags::Use_as_surface) == flags) {
		num_channels = 2;
	}

	auto image = image_cache_.load(filename, num_channels);
	if (!image) {
		return nullptr;
	}

	if (Image::Type::Byte_1 == image->description().type) {
		return std::make_shared<Texture_2D_byte_1_unorm>(image);
	} else if (Image::Type::Byte_2 == image->description().type) {
		return std::make_shared<Texture_2D_byte_2_unorm>(image);
	} else if (Image::Type::Byte_3 == image->description().type) {
		if (static_cast<uint32_t>(Provider::Flags::Use_as_normal) == flags) {
			return std::make_shared<Texture_2D_byte_3_snorm>(image);
		} else if (static_cast<uint32_t>(Provider::Flags::Use_as_surface) == flags) {
			return std::make_shared<Texture_2D_byte_3_unorm>(image);
		} else {
			return std::make_shared<Texture_2D_byte_3_sRGB>(image);
		}
	} else if (Image::Type::Float_3 == image->description().type) {
		return std::make_shared<Texture_2D_float_3>(image);
	}

	return nullptr;//std::make_shared<Texture_2D>(image);
}

}}
