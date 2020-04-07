#ifndef SU_CORE_IMAGE_ENCODING_JSON_READER_HPP
#define SU_CORE_IMAGE_ENCODING_JSON_READER_HPP

#include <iosfwd>
#include <string>

namespace image {

class Image;

namespace encoding::json {

class Reader {
  public:
    static Image* read(std::istream& stream, std::string const& filename);

  private:
};

}  // namespace encoding::json
}  // namespace image

#endif
