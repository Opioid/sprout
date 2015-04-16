#include "image_provider.hpp"
#include "image4.hpp"
#include "image/encoding/png/png_reader.hpp"
#include "image/encoding/rgbe/rgbe_reader.hpp"
#include "base/file/file.hpp"
#include "base/math/vector.inl"
#include <fstream>

namespace image  {

std::shared_ptr<Image> Provider::load(const std::string& filename, uint32_t flags) {
	std::ifstream stream(filename, std::ios::binary);
	if (!stream) {
		throw std::runtime_error("File \"" + filename + "\" could not be opened");
	}

	file::Type type = file::query_type(stream);

	if (file::Type::PNG == type) {
		encoding::png::Reader reader;
		return reader.read(stream);
	} else if (file::Type::RGBE == type) {
		encoding::rgbe::Reader reader;
		return reader.read(stream);
	}

	return nullptr;
}

}
