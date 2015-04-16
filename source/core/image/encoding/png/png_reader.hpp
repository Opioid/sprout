#pragma once

#include <istream>
#include <memory>

namespace image {

class Image;

namespace encoding { namespace png {

class Reader {
public:

	std::shared_ptr<Image> read(std::istream& stream) const;
};

}}}
