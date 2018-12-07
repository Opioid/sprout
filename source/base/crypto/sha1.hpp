#ifndef SU_BASE_CRYPTO_SHA1_HPP
#define SU_BASE_CRYPTO_SHA1_HPP

#include <string>
#include <vector>

namespace crypto::sha1 {

std::vector<uint8_t> encode(std::string const& input);

class SHA1 {
  public:
    SHA1();

    ~SHA1();

    void update(std::string const& s);
    void update(std::istream& is);

    std::vector<uint8_t> final();

  private:
    static uint32_t constexpr Digest_ints = 5;   // number of 32bit integers per SHA1 digest
    static uint32_t constexpr Block_ints  = 16;  // number of 32bit integers per SHA1 block
    static uint32_t constexpr Block_bytes = Block_ints * 4;

    void reset();

    void transform(uint32_t block[Block_bytes]);

    static void buffer_to_block(std::string const& buffer, uint32_t block[Block_bytes]);

    static void read(std::istream& is, std::string& s, size_t max);

    uint64_t transforms_;

    uint32_t digest_[Digest_ints];

    std::string buffer_;
};

}  // namespace crypto::sha1

#endif
