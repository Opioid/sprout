#include "png_writer.hpp"
#include "image/typed_image.inl"
#include "base/math/vector4.inl"
#include "base/thread/thread_pool.hpp"
#include "miniz/miniz.hpp"
#include <fstream>

namespace image { namespace encoding { namespace png {

Writer::Writer(int2 dimensions) : Srgb_alpha(dimensions) {}

std::string Writer::file_extension() const {
	return "png";
}

bool Writer::write(std::ostream& stream, const Float_3& image, thread::Pool& pool) {
	const auto d = image.description().dimensions;
	pool.run_range([this, &image](uint32_t /*id*/, int32_t begin, int32_t end) {
		to_sRGB(image, begin, end); }, 0, d[0] * d[1]);

	size_t buffer_len = 0;
	void* png_buffer = tdefl_write_image_to_png_file_in_memory(rgba_, d[0], d[1], 4, &buffer_len);

	if (!png_buffer) {
		return false;
	}

	stream.write(static_cast<char*>(png_buffer), buffer_len);

	mz_free(png_buffer);

	return true;
}

bool Writer::write(std::ostream& stream, const Float_4& image, thread::Pool& pool) {
	const auto d = image.description().dimensions;
	pool.run_range([this, &image](uint32_t /*id*/, int32_t begin, int32_t end) {
		to_sRGB(image, begin, end); }, 0, d[0] * d[1]);

	size_t buffer_len = 0;
	void* png_buffer = tdefl_write_image_to_png_file_in_memory(rgba_, d[0], d[1], 4, &buffer_len);

	if (!png_buffer) {
		return false;
	}

	stream.write(static_cast<char*>(png_buffer), buffer_len);

	mz_free(png_buffer);

	return true;
}

bool Writer::write(const std::string& name, const Byte_3& image) {
	std::ofstream stream(name, std::ios::binary);
	if (!stream) {
		return false;
	}

	const auto d = image.description().dimensions;

	size_t buffer_len = 0;
	void* png_buffer = tdefl_write_image_to_png_file_in_memory(image.data(), d[0], d[1],
															   3, &buffer_len);

	if (!png_buffer) {
		return false;
	}

	stream.write(static_cast<char*>(png_buffer), buffer_len);

	mz_free(png_buffer);

	return true;
}

bool Writer::write(const std::string& name, const Byte_1& image) {
	std::ofstream stream(name, std::ios::binary);
	if (!stream) {
		return false;
	}

	const auto d = image.description().dimensions;

	size_t buffer_len = 0;
	void* png_buffer = tdefl_write_image_to_png_file_in_memory(image.data(), d[0], d[1],
															   1, &buffer_len);

	if (!png_buffer) {
		return false;
	}

	stream.write(static_cast<char*>(png_buffer), buffer_len);

	mz_free(png_buffer);

	return true;
}

}}}
