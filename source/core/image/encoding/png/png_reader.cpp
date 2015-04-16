#include "png_reader.hpp"
#include "image/image3.hpp"

namespace image { namespace encoding { namespace png {

std::shared_ptr<Image> Reader::read(std::istream& stream) const {
	return std::make_shared<Image3>(Description(math::uint2(256, 256)));
}

}}}
