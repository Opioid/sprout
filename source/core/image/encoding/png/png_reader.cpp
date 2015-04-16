#include "png_reader.hpp"
#include "image/image3.hpp"

namespace image { namespace encoding { namespace png {

std::shared_ptr<Image> Reader::read(std::istream& stream) const {
	auto image = std::make_shared<Image3>(Description(math::uint2(2, 2)));

	image->set3(0, math::float3(0.1f, 0.1f, 0.1f));
	image->set3(1, math::float3(0.3f, 0.3f, 0.3f));
	image->set3(2, math::float3(0.3f, 0.3f, 0.3f));
	image->set3(3, math::float3(0.1f, 0.1f, 0.1f));

	return image;
}

}}}
