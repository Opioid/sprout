#include "image_provider.hpp"
#include "image/encoding/rgbe/rgbe_reader.hpp"
#include "file/file.hpp"
#include "base/math/vector.inl"

namespace image  {

Provider::Provider(thread::Pool& pool) : png_reader_(pool) {}

std::shared_ptr<Image> Provider::load(std::istream& stream, uint32_t flags) {
	file::Type type = file::query_type(stream);

	if (file::Type::PNG == type) {
		return png_reader_.read(stream, static_cast<uint32_t>(Provider::Flags::Use_as_normal) == flags,
										static_cast<uint32_t>(Provider::Flags::Use_as_mask) == flags);
	} else if (file::Type::RGBE == type) {
		encoding::rgbe::Reader reader;
		return reader.read(stream);
	}

	return nullptr;
}

}
