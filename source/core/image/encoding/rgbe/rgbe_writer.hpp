#pragma once

#include "image/image_writer.hpp"

namespace image { namespace encoding { namespace rgbe {

class Writer : public image::Writer {
public:

//	Writer(math::uint2 dimensions);

	virtual std::string file_extension() const final override;

	virtual bool write(std::ostream& stream, const Image_float_4& image,
					   thread::Pool& pool) final override;

private:

	static void write_header(std::ostream& stream, int2 dimensions);

	static void write_pixels(std::ostream& stream, const Image_float_4& image);

	static void write_pixels_rle(std::ostream& stream, const Image_float_4& image);

	static void write_bytes_rle(std::ostream& stream, const uint8_t* data, uint32_t num_bytes);

	static math::byte4 float_to_rgbe(float3_p c);
};

}}}
