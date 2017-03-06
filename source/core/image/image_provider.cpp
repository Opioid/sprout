#include "image_provider.hpp"
#include "resource/resource_manager.hpp"
#include "resource/resource_provider.inl"
#include "encoding/raw/raw_reader.hpp"
#include "encoding/rgbe/rgbe_reader.hpp"
#include "file/file.hpp"
#include "file/file_system.hpp"
#include "base/math/vector3.inl"
#include "base/memory/variant_map.inl"

namespace image  {

Provider::Provider() : resource::Provider<Image>("Image") {}

Provider::~Provider() {}

std::shared_ptr<Image> Provider::load(const std::string& filename,
									  const memory::Variant_map& options,
									  resource::Manager& manager) {
	if ("proc:flakes" == filename) {
		return flakes_provider_.create_normal_map(options);
	} else if ("proc:flakes_mask" == filename) {
		return flakes_provider_.create_mask(options);
	}

	auto stream_pointer = manager.file_system().read_stream(filename);

	auto& stream = *stream_pointer;

	file::Type type = file::query_type(stream);

	if (file::Type::PNG == type) {
		Channels channels = Channels::None;
		options.query("channels", channels);

		int32_t num_elements = 1;
		options.query("num_elements", num_elements);

		return png_reader_.read(stream, channels, num_elements);
	} else if (file::Type::RGBE == type) {
		encoding::rgbe::Reader reader;
		return reader.read(stream);
	} else if (file::Type::Unknown == type) {
		encoding::raw::Reader reader;
		return reader.read(stream);
	}

	throw std::runtime_error("Image type for \"" + filename + "\" not recognized");
}

std::shared_ptr<Image> Provider::load(const void* /*data*/,
									  const std::string& /*mount_folder*/,
									  const memory::Variant_map& /*options*/,
									  resource::Manager& /*manager*/) {
	return nullptr;
}

size_t Provider::num_bytes() const {
	return sizeof(*this);
}

}
