#ifndef SU_CORE_IMAGE_ENCODING_RAW_READER_HPP
#define SU_CORE_IMAGE_ENCODING_RAW_READER_HPP

#include "image/typed_image.hpp"
#include <iosfwd>
#include <memory>

namespace image::encoding::raw {

class Reader {

public:

	std::shared_ptr<Image> read(std::istream& stream) const;

private:


};

}

#endif
