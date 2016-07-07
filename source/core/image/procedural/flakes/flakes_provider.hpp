#pragma once

#include <memory>

namespace memory { class Variant_map; }

namespace image {

class Image;

namespace procedural { namespace flakes {

class Provider {

public:

	std::shared_ptr<Image> create(const memory::Variant_map& options);
};

}}}
