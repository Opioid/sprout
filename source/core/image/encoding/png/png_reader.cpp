#include "png_reader.hpp"
#include "image/typed_image.inl"
#include "image/tiled_image.inl"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "base/string/string.inl"
#include <cstring>
#include <istream>

namespace image { namespace encoding { namespace png {

Reader::Reader() {}

std::shared_ptr<Image> Reader::read(std::istream& stream, Channels channels, int32_t num_elements) {
	std::array<uint8_t, Signature_size> signature;

	stream.read(reinterpret_cast<char*>(signature.data()), sizeof(signature));

	if (Signature != signature) {
		throw std::runtime_error("Bad PNG signature");
	}

	Chunk chunk;
	Info info;

	for (;;) {
		read_chunk(stream, chunk);

		if (!handle_chunk(chunk, info))	{
			break;
		}
	}

	mz_inflateEnd(&info.stream);

	return create_image(info, channels, num_elements);
}

std::shared_ptr<Image> Reader::create_image(const Info& info, Channels channels,
											int32_t num_elements) {
	if (0 == info.num_channels || Channels::None == channels) {
		return nullptr;
	}

	uint32_t num_channels;

	switch (channels) {
	case Channels::X:
	case Channels::Y:
	case Channels::Z:
	case Channels::W:
	default:
		num_channels = 1;
		break;
	case Channels::XY:
		num_channels = 2;
		break;
	case Channels::XYZ:
		num_channels = 3;
		break;
	}

	int2 dimensions;

	if (1 == num_elements) {
		dimensions[0] = info.width;
		dimensions[1] = info.height;
	} else {
		dimensions[0] = info.width;
		dimensions[1] = info.height / num_elements;
	}

	if (1 == num_channels) {
		std::shared_ptr<Byte_1> image = std::make_shared<Byte_1>(
					Image::Description(Image::Type::Byte_1, dimensions, num_elements));

		int32_t c;

		switch (channels) {
		case Channels::X:
		default:
			c = 0;
			break;
		case Channels::Y:
			c = 1;
			break;
		case Channels::Z:
			c = 2;
			break;
		case Channels::W:
			c = 3;
			break;
		}

		if (c >= info.num_channels) {
			c = 0;
		}

		for (int32_t i = 0, len = info.width * info.height; i < len; ++i) {
			int32_t o = i * info.num_channels;

			image->at(i) = info.buffer[o + c];
		}

		return image;
	} else if (2 == num_channels) {
		std::shared_ptr<Byte_2> image = std::make_shared<Byte_2>(
					Image::Description(Image::Type::Byte_2, dimensions, num_elements));

		byte2 color(0, 0);

		int32_t max_channels = std::min(2, info.num_channels);

		for (int32_t i = 0, len = info.width * info.height; i < len; ++i) {
			int32_t o = i * info.num_channels;
			for (int32_t c = 0; c < max_channels; ++c) {
				color.v[c] = info.buffer[o + c];
			}

			image->at(i) = color;
		}

		return image;
	} else if (3 == num_channels) {
		std::shared_ptr<Byte_3> image = std::make_shared<Byte_3>(
					Image::Description(Image::Type::Byte_3, dimensions, num_elements));

		byte3 color(0, 0, 0);

		int32_t max_channels = std::min(3, info.num_channels);

		for (int32_t i = 0, len = info.width * info.height; i < len; ++i) {
			int32_t o = i * info.num_channels;
			for (int32_t c = 0; c < max_channels; ++c) {
				color.v[c] = info.buffer[o + c];
			}

			image->at(i) = color;
		}

		return image;
	}

	return nullptr;
}

void Reader::read_chunk(std::istream& stream, Chunk& chunk) {
	uint32_t length;
	stream.read(reinterpret_cast<char*>(&length), sizeof(uint32_t));
	chunk.length = byteswap(length);

	chunk.type.resize(chunk.length + 4);
	chunk.data = chunk.type.data() + 4;

	stream.read(reinterpret_cast<char*>(chunk.type.data()), chunk.length + 4);

	uint32_t crc;
	stream.read(reinterpret_cast<char*>(&crc), sizeof(uint32_t));
	chunk.crc = byteswap(crc);
}

bool Reader::handle_chunk(const Chunk& chunk, Info& info) {
	const char* type = reinterpret_cast<const char*>(chunk.type.data());

	if (!strncmp("IHDR", type, 4)) {
		return parse_header(chunk, info);
	} else if (!strncmp("PLTE", type, 4)) {
		return parse_lte(chunk, info);
	} else if (!strncmp("IDAT", type, 4)) {
		return parse_data(chunk, info);
	} else if (!strncmp("IEND", type, 4)) {
		return false;
	}

	return true;
}

bool Reader::parse_header(const Chunk& chunk, Info& info) {
	info.width  = byteswap(reinterpret_cast<uint32_t*>(chunk.data)[0]);
	info.height = byteswap(reinterpret_cast<uint32_t*>(chunk.data)[1]);

	uint32_t depth = static_cast<uint32_t>(chunk.data[8]);
	if (8 != depth) {
		throw std::runtime_error(string::to_string(depth) +  " bit depth PNG not supported");
	}

	Color_type color_type = static_cast<Color_type>(chunk.data[9]);

	switch (color_type) {
	case Color_type::Grayscale:
		info.num_channels = 1; break;
	case Color_type::Truecolor:
		info.num_channels = 3; break;
	case Color_type::Truecolor_alpha:
		info.num_channels = 4; break;
	default:
		info.num_channels = 0;
	}

	if (0 == info.num_channels) {
		return false;
	}

	info.bytes_per_pixel = info.num_channels;

	uint8_t interlace = chunk.data[12];
	if (interlace) {
		// currently don't support interlaced encoding
		return false;
	}

	info.buffer.resize(info.width * info.height * info.num_channels);

	info.current_filter = Filter::None;
	info.filter_byte = true;
	info.current_byte = 0;
	info.current_byte_total = 0;

	info.current_row_data.resize(info.width * info.num_channels);
	info.previous_row_data.resize(info.current_row_data.size());

	info.stream.zalloc = nullptr;
	info.stream.zfree  = nullptr;
	if (MZ_OK != mz_inflateInit(&info.stream)) {
		return false;
	}

	return true;
}

bool Reader::parse_lte(const Chunk& /*chunk*/, Info& /*info*/) {
	return true;
}

bool Reader::parse_data(const Chunk& chunk, Info& info) {
	const uint32_t buffer_size = 8192;
	uint8_t buffer[buffer_size];

	info.stream.next_in = chunk.data;
	info.stream.avail_in = chunk.length;

	do {
		info.stream.next_out = buffer;
		info.stream.avail_out = buffer_size;

		int status = mz_inflate(&info.stream, MZ_NO_FLUSH);
		if (status != MZ_OK && status != MZ_STREAM_END
			&& status != MZ_BUF_ERROR && status != MZ_NEED_DICT) {
			return false;
		}

		uint32_t decompressed = buffer_size - info.stream.avail_out;

		for (uint32_t i = 0; i < decompressed; ++i) {
			if (info.filter_byte) {
				info.current_filter = static_cast<Filter>(buffer[i]);
				info.filter_byte = false;
			} else {
				uint8_t raw = filter(buffer[i], info.current_filter, info);
				info.current_row_data[info.current_byte] = raw;
				info.buffer[info.current_byte_total++] = raw;

				if (info.current_row_data.size() - 1 == info.current_byte) {
					info.current_byte = 0;
					std::swap(info.current_row_data, info.previous_row_data);
					info.filter_byte = true;
				} else {
					++info.current_byte;
				}
			}
		}
	} while (info.stream.avail_in > 0 || 0 == info.stream.avail_out);

	return true;
}

uint8_t Reader::filter(uint8_t byte, Filter filter, const Info& info) {
	switch (filter) {
	case Filter::None:
		return byte;
	case Filter::Sub:
		return byte + raw(info.current_byte - info.bytes_per_pixel, info);
	case Filter::Up:
		return byte + prior(info.current_byte, info);
	case Filter::Average:
		return byte + average(raw(info.current_byte - info.bytes_per_pixel, info),
							  prior(info.current_byte, info));
	case Filter::Paeth:
		return byte + paeth_predictor(raw(info.current_byte - info.bytes_per_pixel, info),
									  prior(info.current_byte, info),
									  prior(info.current_byte - info.bytes_per_pixel, info));
	default:
		return 0;
	}
}

uint8_t Reader::raw(int column, const Info& info) {
	if (column < 0) {
		return 0;
	}

	return info.current_row_data[column];
}

uint8_t Reader::prior(int column, const Info& info) {
	if (column < 0) {
		return 0;
	}

	return info.previous_row_data[column];
}

uint8_t Reader::average(uint8_t a, uint8_t b) {
	return (static_cast<uint32_t>(a) + static_cast<uint32_t>(b)) >> 1;
}

uint8_t Reader::paeth_predictor(uint8_t a, uint8_t b, uint8_t c) {
	int A = static_cast<int>(a);
	int B = static_cast<int>(b);
	int C = static_cast<int>(c);
	int p = A + B - C;
	int pa = std::abs(p - A);
	int pb = std::abs(p - B);
	int pc = std::abs(p - C);

	if (pa <= pb && pa <= pc) {
		return a;
	}

	if (pb <= pc) {
		return b;
	}

	return c;
}

uint32_t Reader::byteswap(uint32_t v) {
	return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8)
		 | ((v & 0xFF0000) >> 8) | ((v & 0xFF000000) >> 24);
}

const std::array<uint8_t, Reader::Signature_size> Reader::Signature = {
	{ 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A } };

}}}
