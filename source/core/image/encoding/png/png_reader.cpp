#include "png_reader.hpp"
#include "image/image3.hpp"
#include "miniz/miniz.c"
#include <iostream>

namespace image { namespace encoding { namespace png {

std::shared_ptr<Image> Reader::read(std::istream& stream) const {
	std::array<uint8_t, Signature_size> signature;

	stream.read(reinterpret_cast<char*>(signature.data()), sizeof(signature));

	if (Signature != signature) {
		throw std::runtime_error("Bad PNG signature");
	}

	Info info;

	for (;;) {
		auto chunk = read_chunk(stream);

		if (!handle_chunk(chunk, info))	{
			break;
		}
	}

	auto image = std::make_shared<Image3>(Description(math::uint2(2, 2)));

	image->set3(0, math::float3(0.1f, 0.1f, 0.1f));
	image->set3(1, math::float3(0.3f, 0.3f, 0.3f));
	image->set3(2, math::float3(0.3f, 0.3f, 0.3f));
	image->set3(3, math::float3(0.1f, 0.1f, 0.1f));

	return image;
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

	return false;
}

bool Reader::parse_header(std::shared_ptr<Chunk> chunk, Info& info) {
	info.width  = swap(reinterpret_cast<uint32_t*>(chunk->data)[0]);
	info.height = swap(reinterpret_cast<uint32_t*>(chunk->data)[1]);


	if (MZ_OK != mz_inflateInit(&info.stream)) {
		return false;
	}

	return true;
}

bool Reader::parse_lte(std::shared_ptr<Chunk> chunk, Info& info) {
	std::cout << "parse_lte" << std::endl;
	return true;
}

bool Reader::parse_data(std::shared_ptr<Chunk> chunk, Info& info) {
	std::cout << "parse_data" << std::endl;
	return true;
}

uint32_t Reader::swap(uint32_t v) {
	return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) | ((v & 0xFF000000) >> 24);
}

const std::array<uint8_t, Reader::Signature_size> Reader::Signature = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

}}}
