#include "image_provider.hpp"
#include "resource/resource_provider.inl"
#include "image/encoding/rgbe/rgbe_reader.hpp"
#include "file/file.hpp"
#include "file/file_system.hpp"
#include "base/math/vector.inl"

namespace image  {

Provider::Provider(file::System& file_system, thread::Pool& thread_pool) :
	resource::Provider<Image>(file_system, thread_pool) {}

std::shared_ptr<Image> Provider::load(const std::string& filename, uint32_t flags) {
	auto stream_pointer = file_system_.read_stream(filename);
	if (!*stream_pointer) {
		throw std::runtime_error("File \"" + filename + "\" could not be opened");
	}

	auto& stream = *stream_pointer;

	file::Type type = file::query_type(stream);

	if (file::Type::PNG == type) {
		return png_reader_.read(stream, flags);
	} else if (file::Type::RGBE == type) {
		encoding::rgbe::Reader reader;
		return reader.read(stream);
	}

	return nullptr;
}

}
