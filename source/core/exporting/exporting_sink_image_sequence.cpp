#include "exporting_sink_image_sequence.hpp"
#include "base/string/string.hpp"
#include "image/image_writer.hpp"

#include <fstream>

namespace exporting {

static char const* AOV_names[] = {"_albedo", "_roughness", "_gn", "_sn", "_mat_id", ""};

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

    using namespace rendering::sensor;

    if (aov::Property::Unknown == aov) {
        writer_->write(stream, image, threads);
    } else {
        bool const is_data = aov::is_data(aov);

        writer_->write(stream, image, is_data, threads);
    }
}

}  // namespace exporting
