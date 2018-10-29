#ifndef SU_BASE_CRYPTO_BASE64_HPP
#define SU_BASE_CRYPTO_BASE64_HPP

#include <string>

namespace crypto::base64 {

std::string encode(uint8_t const* data, size_t size);
}

#endif
