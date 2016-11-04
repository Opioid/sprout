#pragma once

#include <string>
#include <vector>

namespace crypto { namespace sha1 {

std::vector<uint8_t> encode(const std::string& input);

class SHA1 {

public:

	SHA1();
	~SHA1();

	void update(const std::string& s);
	void update(std::istream& is);

	std::vector<uint8_t> final();

private:

	static constexpr uint32_t Digest_ints = 5;  // number of 32bit integers per SHA1 digest
	static constexpr uint32_t Block_ints = 16;  // number of 32bit integers per SHA1 block
	static constexpr uint32_t Block_bytes = Block_ints * 4;

	void reset();
	void transform(uint32_t block[Block_bytes]);

	static void buffer_to_block(const std::string& buffer, uint32_t block[Block_bytes]);
	static void read(std::istream& is, std::string& s, size_t max);

	uint64_t transforms_;

	uint32_t digest_[Digest_ints];

	std::string buffer_;
};

}}
