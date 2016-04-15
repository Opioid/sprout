#include "png_writer.hpp"
#include "base/math/vector.inl"
#include "base/thread/thread_pool.hpp"
#include "miniz/miniz.hpp"

namespace image { namespace encoding { namespace png {

Writer::Writer(math::int2 dimensions) : Srgb(dimensions) {}

std::string Writer::file_extension() const {
	return "png";
}

bool Writer::write(std::ostream& stream, const Image_float_4& image, thread::Pool& pool) {
	auto d = image.description().dimensions;
	pool.run_range([this, &image](uint32_t begin, uint32_t end){
		to_sRGB(image, begin, end); }, 0, d.x * d.y);

	size_t buffer_len = 0;
	void* png_buffer = tdefl_write_image_to_png_file_in_memory(rgba_, d.x, d.y, 4, &buffer_len);

	if (!png_buffer) {
		return false;
	}

	stream.write(static_cast<char*>(png_buffer), buffer_len);

	mz_free(png_buffer);

	return true;
}

}}}
