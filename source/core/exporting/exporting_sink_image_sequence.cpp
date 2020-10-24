#include "exporting_sink_image_sequence.hpp"
#include "base/string/string.hpp"
#include "image/image_writer.hpp"

#include <fstream>

namespace exporting {

static char const* AOV_names[] = {"_gn", "_sn", ""};

Image_sequence::Image_sequence(std::string filename, image::Writer* writer)
    : filename_(std::move(filename)), writer_(writer) {}

Image_sequence::~Image_sequence() {
    delete writer_;
}

void Image_sequence::write(image::Float4 const& image, AOV aov, uint32_t frame, Threads& threads) {
    std::ofstream stream(filename_ + string::to_string(frame, 6) +
                             std::string(AOV_names[uint32_t(aov)]) + "." +
                             writer_->file_extension(),
                         std::ios::binary);
    if (!stream) {
        return;
    }

    writer_->write(stream, image, threads);
}

}  // namespace exporting
