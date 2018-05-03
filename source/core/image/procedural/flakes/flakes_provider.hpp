#pragma once

#include "base/math/vector2.hpp"
#include <memory>

namespace memory { class Variant_map; }

namespace image {

class Image;

namespace procedural { namespace flakes {

class Provider {

public:

	std::shared_ptr<Image> create_normal_map(memory::Variant_map const& options);

	std::shared_ptr<Image> create_mask(memory::Variant_map const& options);

private:

	struct Properties {
		Properties(memory::Variant_map const& options);

		int2 dimensions;

		uint32_t num_flakes;
		float	 radius;
		float	 variance;
	};
};

}}}
