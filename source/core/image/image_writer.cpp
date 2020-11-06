#include "image_writer.hpp"

namespace image {

Writer::~Writer() = default;

bool Writer::write(std::ostream& stream, Float4 const& image, Encoding /*encoding*/,
                   Threads& threads) {
    return write(stream, image, threads);
}

}  // namespace image
