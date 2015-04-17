#pragma once

// based on
// https://github.com/jansol/LuPng

#include "miniz/miniz.h"
#include <cstdint>
#include <istream>
#include <memory>
#include <array>

namespace image {

class Image;

namespace encoding { namespace png {

class Reader {
public:

	std::shared_ptr<Image> read(std::istream& stream) const;

private:

	struct Chunk {
		~Chunk();

		uint32_t length;
		uint8_t* type;
		uint8_t* data;
		uint32_t crc;
	};

	struct Info {
		// header
		uint32_t width;
		uint32_t height;

		// miniz
		mz_stream stream;
	};

	static std::shared_ptr<Chunk> read_chunk(std::istream& stream);

	static bool handle_chunk(std::shared_ptr<Chunk> chunk, Info& info);

	static bool parse_header(std::shared_ptr<Chunk> chunk, Info& info);

	static bool parse_lte(std::shared_ptr<Chunk> chunk, Info& info);

	static bool parse_data(std::shared_ptr<Chunk> chunk, Info& info);

	static uint32_t swap(uint32_t v);

	static const uint32_t Signature_size = 8;

	static const std::array<uint8_t, Signature_size> Signature;
};

}}}
