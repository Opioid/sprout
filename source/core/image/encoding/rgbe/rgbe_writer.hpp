#pragma once

#include "image/image_writer.hpp"

namespace image { namespace encoding { namespace rgbe {

class Writer : public image::Writer {
public:

//	Writer(math::uint2 dimensions);

	virtual std::string file_extension() const override final;

	virtual bool write(std::ostream& stream, const Float4& image,
					   thread::Pool& pool) override final;

private:

	static void write_header(std::ostream& stream, int2 dimensions);

	static void write_pixels(std::ostream& stream, const Float4& image);

	static void write_pixels_rle(std::ostream& stream, const Float4& image);

	static void write_bytes_rle(std::ostream& stream, const uint8_t* data, uint32_t num_bytes);

	static byte4 float_to_rgbe(const float4& c);
};

}}}
