#include "png_reader.hpp"
#include "image/image3.hpp"
#include "base/color/color.hpp"
#include "base/math/vector.inl"
#include "image/image_writer.hpp"
#include <cstring>
#include <iostream>

namespace image { namespace encoding { namespace png {

std::shared_ptr<Image> Reader::read(std::istream& stream, bool use_as_normal) const {
	std::array<uint8_t, Signature_size> signature;

	stream.read(reinterpret_cast<char*>(signature.data()), sizeof(signature));

	if (Signature != signature) {
		throw std::runtime_error("Bad PNG signature");
	}

	Info info;
	info.use_as_normal = use_as_normal;

	for (;;) {
		auto chunk = read_chunk(stream);

		if (!handle_chunk(chunk, info))	{
			break;
		}
	}

	mz_inflateEnd(&info.stream);

	return info.image;
}

Reader::Chunk::~Chunk() {
	delete [] type;
}

std::shared_ptr<Reader::Chunk> Reader::read_chunk(std::istream& stream) {
	auto chunk = std::make_shared<Chunk>();

	stream.read(reinterpret_cast<char*>(&chunk->length), sizeof(uint32_t));
	chunk->length = swap(chunk->length);

	chunk->type = new uint8_t[chunk->length + 4];
	chunk->data = chunk->type + 4;

	stream.read(reinterpret_cast<char*>(chunk->type), chunk->length + 4);

	stream.read(reinterpret_cast<char*>(&chunk->crc), sizeof(uint32_t));
	chunk->crc = swap(chunk->crc);

	return chunk;
}

bool Reader::handle_chunk(std::shared_ptr<Chunk> chunk, Info& info) {
	const char* type = reinterpret_cast<char*>(chunk->type);

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

bool Reader::parse_header(std::shared_ptr<Chunk> chunk, Info& info) {
	info.width  = swap(reinterpret_cast<uint32_t*>(chunk->data)[0]);
	info.height = swap(reinterpret_cast<uint32_t*>(chunk->data)[1]);

	uint8_t depth = chunk->data[8];
	if (8 != depth) {
		return false;
	}

	Color_type color_type = static_cast<Color_type>(chunk->data[9]);

	switch (color_type) {
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

	uint8_t interlace = chunk->data[12];
	if (interlace) {
		// currently don't support interlaced encoding
		return false;
	}

	info.image = std::make_shared<Image3>(Description(math::uint2(info.width, info.height)));

	info.current_filter = Filter::None;
	info.filter_byte = true;
	info.current_byte = 0;
	info.channel = 0;
	info.current_pixel = 0;

	info.current_row_data.resize(info.width * info.num_channels);
	info.previous_row_data.resize(info.current_row_data.size());

	info.stream.zalloc = nullptr;
	info.stream.zfree  = nullptr;
	if (MZ_OK != mz_inflateInit(&info.stream)) {
		return false;
	}

	return true;
}

bool Reader::parse_lte(std::shared_ptr<Chunk> /*chunk*/, Info& /*info*/) {
	return true;
}

bool Reader::parse_data(std::shared_ptr<Chunk> chunk, Info& info) {
	const uint32_t buffer_size = 8192;
	uint8_t buffer[buffer_size];

	info.stream.next_in = chunk->data;
	info.stream.avail_in = chunk->length;

	color::Color4c color(0, 0, 0, 255);

	do {
		info.stream.next_out = buffer;
		info.stream.avail_out = buffer_size;

		int status = mz_inflate(&info.stream, MZ_NO_FLUSH);

		if (status != MZ_OK && status != MZ_STREAM_END && status != MZ_BUF_ERROR && status != MZ_NEED_DICT) {
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
				color.v[info.channel] = raw;

				if (info.num_channels - 1 == info.channel) {
					math::float4 linear;
					if (info.use_as_normal) {
						linear.x = 2.f * (static_cast<float>(color.x) / 255.f - 0.5f);
						linear.y = 2.f * (static_cast<float>(color.y) / 255.f - 0.5f);
						linear.z = 2.f * (static_cast<float>(color.z) / 255.f - 0.5f);
					} else {
						linear = color::sRGB_to_linear(color);
					}

					info.image->set4(info.current_pixel++, linear);
					info.channel = 0;
				} else {
					++info.channel;
				}

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

uint32_t Reader::swap(uint32_t v) {
	return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) | ((v & 0xFF000000) >> 24);
}

const std::array<uint8_t, Reader::Signature_size> Reader::Signature = { { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A } };

}}}
