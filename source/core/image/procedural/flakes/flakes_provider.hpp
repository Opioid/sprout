#pragma once

#include <memory>

namespace memory { class Variant_map; }

namespace image {

class Image;

namespace procedural { namespace flakes {

class Provider {

public:

	std::shared_ptr<Image> create_normal_map(const memory::Variant_map& options);

	std::shared_ptr<Image> create_mask(const memory::Variant_map& options);
};

}}}
