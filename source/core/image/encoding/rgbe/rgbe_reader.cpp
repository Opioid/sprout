#include "rgbe_reader.hpp"
#include "image/typed_image.inl"
#include "base/math/vector4.inl"
#include <cmath>
#include <istream>
#include <string>
#include <vector>

// http://www.graphics.cornell.edu/~bjw/rgbe

namespace image { namespace encoding { namespace rgbe {

std::shared_ptr<Image> Reader::read(std::istream& stream) const {
	const Header header = read_header(stream);

	const int2 dimensions(header.width, header.height);

	auto image = std::make_shared<Float3>(
				Image::Description(Image::Type::Float3, dimensions));

	read_pixels_RLE(stream, header.width, header.height, *image);

	return image;
}

Reader::Header Reader::read_header(std::istream& stream) {
	std::string line;
	std::getline(stream, line);
	if ("#?" != line.substr(0, 2)) {
		throw std::runtime_error("Bad initial token");
	}

	bool format_specifier = false;

	for (;;) {
		std::getline(stream, line);

		if (line.empty() || 0 == line[0]) {
			// blank lines signifies end of meta data header
			break;
		} else if ("FORMAT=32-bit_rle_rgbe" == line) {
			format_specifier = true;
		}
	}

	if (!format_specifier) {
		throw std::runtime_error("No FORMAT specifier found");
	}

	Header header;

	std::getline(stream, line);

	if (std::sscanf(line.c_str(), "-Y %d +X %d", &header.height, &header.width) < 2) {
		throw std::runtime_error("Missing image size specifier");
	}

	return header;
}

void Reader::read_pixels_RLE(std::istream& stream,
							 uint32_t scanline_width, uint32_t num_scanlines,
							 Float3& image) {
	if (scanline_width < 8 || scanline_width > 0x7fff) {
		return read_pixels(stream, scanline_width * num_scanlines, image, 0);
	}

	uint32_t offset = 0;
	uint8_t rgbe[4];
	std::vector<uint8_t> scanline_buffer(4 * scanline_width);
	uint8_t buf[2];

	for (; num_scanlines > 0; --num_scanlines) {
		stream.read(reinterpret_cast<char*>(rgbe), sizeof(rgbe));

		if (rgbe[0] != 2 || rgbe[1] != 2 || (rgbe[2] & 0x80) != 0) {
			// this file is not run length encoded
			image_float3 color = rgbe_to_float3(rgbe);

			image.at(0) = color;

			read_pixels(stream, scanline_width * num_scanlines - 1, image, 1);
			return;
		}

		if ((static_cast<uint32_t>(rgbe[2]) << 8 | static_cast<uint32_t>(rgbe[3]))
				!= scanline_width) {
			throw std::runtime_error("Wrong scanline width");
		}

		// read each of the four channels for the scanline into the buffer
		for (uint32_t i = 0, index = 0; i < 4; ++i) {
			uint32_t end = (i + 1) * scanline_width;

			for (; index < end;) {
				stream.read(reinterpret_cast<char*>(buf), sizeof(buf));

				if (buf[0] > 128) {
					// a run of the same value
					uint32_t count = static_cast<uint32_t>(buf[0]) - 128;

					if (count == 0 || count > end - index) {
						throw std::runtime_error("Bad scanline data");
					}

					for (; count > 0; --count) {
						scanline_buffer[index++] = buf[1];
					}
				} else {
					// a non-run
					uint32_t count = static_cast<uint32_t>(buf[0]);

					if (count == 0 || count > end - index) {
						throw std::runtime_error("Bad scanline data");
					}

					scanline_buffer[index++] = buf[1];

					if (--count > 0) {
						stream.read(reinterpret_cast<char*>(&scanline_buffer[index]), count);

						index += count;
					}
				}
			}
		}

		// now convert data from buffer into floats
		for (uint32_t i = 0; i < scanline_width; ++i) {
			rgbe[0] = scanline_buffer[i];
			rgbe[1] = scanline_buffer[i +     scanline_width];
			rgbe[2] = scanline_buffer[i + 2 * scanline_width];
			rgbe[3] = scanline_buffer[i + 3 * scanline_width];

			image.at(offset++) = rgbe_to_float3(rgbe);
		}
	}
}

void Reader::read_pixels(std::istream& stream, uint32_t num_pixels,
						 Float3& image, uint32_t offset) {
	uint8_t rgbe[4];

	for (; num_pixels > 0; --num_pixels) {
		stream.read(reinterpret_cast<char*>(rgbe), sizeof(rgbe));

		image_float3 color = rgbe_to_float3(rgbe);

		image.at(offset++) = color;
	}
}

Reader::image_float3 Reader::rgbe_to_float3(uint8_t rgbe[4]) {
	if (rgbe[3] > 0) {
		// nonzero pixel
		float f = std::ldexp(1.f, static_cast<int>(rgbe[3]) - (128 + 8));
		return image_float3(static_cast<float>(rgbe[0]) * f,
							static_cast<float>(rgbe[1]) * f,
							static_cast<float>(rgbe[2]) * f);
	} else {
		return image_float3(0.f);
	}
}

}}}
