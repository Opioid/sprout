#include "exporting_sink_image_sequence.hpp"
#include <fstream>
#include "base/math/vector4.inl"
#include "base/string/string.hpp"
#include "image/image_writer.hpp"

namespace exporting {

Image_sequence::Image_sequence(std::string const& filename, std::unique_ptr<image::Writer> writer)
    : filename_(filename), writer_(std::move(writer)) {}

void Image_sequence::write(image::Float4 const& image, uint32_t frame, thread::Pool& pool) {
    std::ofstream stream(filename_ + string::to_string(frame, 6) + "." + writer_->file_extension(),
                         std::ios::binary);
    if (!stream) {
        return;
    }

    writer_->write(stream, image, pool);
}

}  // namespace exporting
