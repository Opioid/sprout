#pragma once

// based on
// https://github.com/jansol/LuPng

#include "miniz/miniz.hpp"
#include <cstdint>
#include <istream>
#include <memory>
#include <array>
#include <vector>

namespace image {

class Image;

namespace encoding { namespace png {

class Reader {
public:

	Reader();

	std::shared_ptr<Image> read(std::istream& stream, uint32_t num_channels);

private:

	struct Chunk {
		uint32_t length;
		std::vector<uint8_t> type;
		uint8_t* data;
		uint32_t crc;
	};

	enum class Color_type {
		Grayscale = 0,
		Truecolor = 2,
		Palleted = 3,
		Grayscale_alpha = 4,
		Truecolor_alpha = 6
	};

	enum class Filter {
		None,
		Sub,
		Up,
		Average,
		Paeth
	};

	struct Info {
		// header
		int32_t width;
		int32_t height;

		int32_t num_channels;
		int32_t bytes_per_pixel;

		std::vector<uint8_t> buffer;

		// parsing state
		Filter current_filter;
		bool filter_byte;
		uint32_t current_byte;
		uint32_t current_byte_total;

		std::vector<uint8_t> current_row_data;
		std::vector<uint8_t> previous_row_data;

		// miniz
		mz_stream stream;
	};

	std::shared_ptr<Image> create_image(const Info& info, int32_t num_channels) const;

	static void read_chunk(std::istream& stream, Chunk& chunk);

	static bool handle_chunk(const Chunk& chunk, Info& info);

	static bool parse_header(const Chunk& chunk, Info& info);

	static bool parse_lte(const Chunk& chunk, Info& info);

	static bool parse_data(const Chunk& chunk, Info& info);

	static uint8_t filter(uint8_t byte, Filter filter, const Info& info);

	static uint8_t raw(int column, const Info& info);
	static uint8_t prior(int column, const Info& info);

	static uint8_t average(uint8_t a, uint8_t b);
	static uint8_t paeth_predictor(uint8_t a, uint8_t b, uint8_t c);

	static uint32_t byteswap(uint32_t v);

	static const uint32_t Signature_size = 8;

	static const std::array<uint8_t, Signature_size> Signature;
};

}}}
