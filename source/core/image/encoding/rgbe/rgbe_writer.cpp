#include "rgbe_writer.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"
#include <cmath>

// http://www.graphics.cornell.edu/~bjw/rgbe

namespace image { namespace encoding { namespace rgbe {

std::string Writer::file_extension() const {
	return "hdr";
}

bool Writer::write(std::ostream& stream, const image::Float_4& image, thread::Pool& /*pool*/) {
	write_header(stream, image.description().dimensions.xy());

	write_pixels_rle(stream, image);

	return true;
}

void Writer::write_header(std::ostream& stream, int2 dimensions) {
	stream << "#?RGBE\n";
	stream << "FORMAT=32-bit_rle_rgbe\n\n";
	stream << "-Y " << dimensions[1] << " +X " << dimensions[0] << "\n";
}

void Writer::write_pixels(std::ostream& stream, const Float_4& image) {
	const auto& d = image.description().dimensions;
	for (uint32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
		math::byte4 rgbe = float_to_rgbe(image.at(i));

		stream.write(reinterpret_cast<char*>(&rgbe), sizeof(math::byte4));
	}
}

void Writer::write_pixels_rle(std::ostream& stream, const Float_4& image) {
	uint32_t scanline_width = image.description().dimensions[0];
	uint32_t num_scanlines  = image.description().dimensions[1];

	if (scanline_width < 8 || scanline_width > 0x7fff) {
		// run length encoding is not allowed so write flat
		return write_pixels(stream, image);
	}

	uint8_t* buffer = new uint8_t[scanline_width * 4];

	uint32_t current_pixel = 0;

	while (num_scanlines-- > 0) {
		math::byte4 rgbe;
		rgbe[0] = 2;
		rgbe[1] = 2;
		rgbe[2] = static_cast<uint8_t>(scanline_width >> 8);
		rgbe[3] = static_cast<uint8_t>(scanline_width & 0xFF);

		stream.write(reinterpret_cast<char*>(&rgbe), sizeof(math::byte4));

		for (uint32_t i = 0; i < scanline_width; ++i, ++current_pixel) {
			const auto& pixel = image.at(current_pixel);

			rgbe = float_to_rgbe(pixel);

			buffer[i]					   = rgbe[0];
			buffer[i + scanline_width]     = rgbe[1];
			buffer[i + scanline_width * 2] = rgbe[2];
			buffer[i + scanline_width * 3] = rgbe[3];
		}

		// write out each of the four channels separately run length encoded
		// first red, then green, then blue, then exponent
		for (uint32_t i = 0; i < 4; ++i) {
			write_bytes_rle(stream, &buffer[i * scanline_width], scanline_width);
		}
	}

	delete[] buffer;
}

// The code below is only needed for the run-length encoded files.
// Run length encoding adds considerable complexity but does
// save some space.  For each scanline, each channel (r,g,b,e) is
// encoded separately for better compression.
void Writer::write_bytes_rle(std::ostream& stream, const uint8_t* data, uint32_t num_bytes) {
	constexpr uint32_t min_run_length = 4;

	uint8_t buffer[2];
	uint32_t current = 0;

	while (current < num_bytes) {
		uint32_t begin_run = current;

		// find next run of length at least 4 if one exists
		uint32_t run_count = 0;
		uint32_t old_run_count = 0;

		while (run_count < min_run_length && begin_run < num_bytes) {
			begin_run += run_count;
			old_run_count = run_count;
			run_count = 1;

			while (begin_run + run_count < num_bytes && run_count < 127
			   &&  data[begin_run] == data[begin_run + run_count]) {
				++run_count;
			}
		}

		// if data before next big run is a short run then write it as such
		if (old_run_count > 1 && old_run_count == begin_run - current) {
			buffer[0] = static_cast<uint8_t>(128 + old_run_count);   // write short run
			buffer[1] = data[current];

			stream.write(reinterpret_cast<char*>(&buffer), sizeof(uint8_t) * 2);

			current = begin_run;
		}

		// write out bytes until we reach the start of the next run
		while (current < begin_run) {
			uint32_t nonrun_count = begin_run - current;

			if (nonrun_count > 128) {
				nonrun_count = 128;
			}

			buffer[0] = static_cast<uint8_t>(nonrun_count);

			stream.write(reinterpret_cast<char*>(&buffer), sizeof(uint8_t));

			stream.write(reinterpret_cast<const char*>(&data[current]),
						 sizeof(uint8_t) * nonrun_count);

			current += nonrun_count;
		}

		// write out next run if one was found
		if (run_count >= min_run_length) {
			buffer[0] = static_cast<uint8_t>(128 + run_count);
			buffer[1] = data[begin_run];

			stream.write(reinterpret_cast<char*>(&buffer), sizeof(uint8_t) * 2);

			current += run_count;
		}
	}
}

math::byte4 Writer::float_to_rgbe(float4_p c) {
	float v = c[0];

	if (c[1] > v) {
		v = c[1];
	}

	if (c[2] > v) {
		v = c[2];
	}

	if (v < 1e-32) {
		return math::byte4(0, 0, 0, 0);
	} else {
		int e;
		float f = std::frexp(v, &e);

		v = f * 256.f / v;

		return math::byte4(static_cast<uint8_t>(c[0] * v),
						   static_cast<uint8_t>(c[1] * v),
						   static_cast<uint8_t>(c[2] * v),
						   static_cast<uint8_t>(e + 128));
	}
}

}}}
