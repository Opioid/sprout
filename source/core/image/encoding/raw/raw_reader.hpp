#pragma once

#include "image/typed_image.hpp"
#include "base/math/vector.hpp"
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <vector>

namespace image { namespace encoding { namespace raw {

class Reader {

public:

	std::shared_ptr<Image> read(std::istream& stream) const;

private:


};

}}}
