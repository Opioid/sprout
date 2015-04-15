#include "image_provider.hpp"
#include "image4.hpp"
#include "image/encoding/rgbe_reader.hpp"
#include "base/math/vector.inl"
#include <fstream>

namespace image  {

std::shared_ptr<Image> Provider::load(const std::string& filename, uint32_t flags) {
	std::ifstream stream(filename, std::ios::binary);
	if (!stream) {
		return nullptr;
	}

	encoding::Rgbe_reader reader;
	return reader.read(stream);
}

}
