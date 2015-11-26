#include "exporting_sink_image_sequence.hpp"
#include "image/image.hpp"
#include "base/color/color.inl"
#include "base/string/string.inl"
#include "base/thread/thread_pool.hpp"
#include "miniz/miniz.hpp"
#include <fstream>

namespace exporting {

Image_sequence::Image_sequence(const std::string& filename, const math::uint2& dimensions) :
	Srgb(dimensions), filename_(filename) {}

Image_sequence::~Image_sequence() {}

void Image_sequence::write(const image::Image_float_4& image, uint32_t frame, thread::Pool& pool) {
	std::ofstream stream(filename_ + string::to_string(frame, 2) + ".png", std::ios::binary);
	if (!stream) {
		return;
	}

	auto d = image.description().dimensions;
	pool.run_range([this, &image](uint32_t begin, uint32_t end){ to_sRGB(image, begin, end); }, 0, d.x * d.y);

	size_t buffer_len = 0;
	void* png_buffer = tdefl_write_image_to_png_file_in_memory(rgba_, d.x, d.y, 4, &buffer_len);

	if (!png_buffer) {
		return;
	}

	stream.write(static_cast<char*>(png_buffer), buffer_len);

	mz_free(png_buffer);
}

}
