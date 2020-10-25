#include "image_writer.hpp"
#include "base/math/vector4.inl"

namespace image {

Writer::~Writer() = default;

bool Writer::write(std::ostream& stream, Float4 const& image, bool /*data*/, Threads& threads) {
    return write(stream, image, threads);
}

}  // namespace image
