#include "texture_2d_provider.hpp"
#include "texture_2d.hpp"
#include "resource/resource_provider.inl"
#include "resource/resource_cache.inl"
#include "image/image_provider.hpp"

namespace image { namespace texture {

Provider::Provider(file::System& file_system, thread::Pool& pool) :
	resource::Provider<Texture_2D>(file_system),
	image_provider_(file_system_, pool),
	image_cache_(image_provider_) {}

std::shared_ptr<Texture_2D> Provider::load(const std::string& filename, uint32_t flags) {
	auto stream_pointer = file_system_.read_stream(filename);
	if (!*stream_pointer) {
		throw std::runtime_error("File \"" + filename + "\" could not be opened");
	}

	auto image = image_cache_.load(filename, flags);

	return std::make_shared<Texture_2D>(image);
}

}}
