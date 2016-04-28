#include "exporting_sink_image_sequence.hpp"
#include "image/image_writer.hpp"
#include "base/math/vector.inl"
#include "base/string/string.inl"
#include <fstream>

namespace exporting {

Image_sequence::Image_sequence(const std::string& filename, image::Writer* writer) :
	filename_(filename), writer_(writer) {}

Image_sequence::~Image_sequence() {
	delete writer_;
}

void Image_sequence::write(const image::Image_float_4& image, uint32_t frame, thread::Pool& pool) {
	std::ofstream stream(filename_ + string::to_string(frame, 6) + "." +
						 writer_->file_extension(), std::ios::binary);
	if (!stream) {
		return;
	}

	writer_->write(stream, image, pool);
}

}
