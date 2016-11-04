#pragma once

#include "base/math/vector.hpp"
#include <memory>

namespace memory { class Variant_map; }

namespace image {

class Image;

namespace procedural { namespace flakes {

class Provider {

public:

	std::shared_ptr<Image> create_normal_map(const memory::Variant_map& options);

	std::shared_ptr<Image> create_mask(const memory::Variant_map& options);

private:

	struct Properties {
		Properties(const memory::Variant_map& options);

		int2 dimensions;

		uint32_t num_flakes;
		float	 radius;
		float	 variance;
	};
};

}}}
