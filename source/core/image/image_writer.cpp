#include "image_writer.hpp"
#include "base/math/vector3.inl"
#include "channels.hpp"

namespace image {

Writer::~Writer() = default;

bool Writer::write(std::ostream& stream, Float4 const& image, Layout /*layout*/, Threads& threads) {
    return write(stream, image, threads);
}

}  // namespace image
