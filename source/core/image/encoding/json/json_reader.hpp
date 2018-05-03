#ifndef SU_CORE_IMAGE_ENCODING_JSON_READER_HPP
#define SU_CORE_IMAGE_ENCODING_JSON_READER_HPP

#include "image/typed_image.hpp"
#include <iosfwd>
#include <memory>

namespace image::encoding::json {

class Reader {

public:

	std::shared_ptr<Image> read(std::istream& stream) const;

private:

};

}

#endif
