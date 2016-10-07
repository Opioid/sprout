#pragma once

#include "image/typed_image.hpp"
#include "base/math/vector.hpp"
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <vector>

namespace image { namespace encoding { namespace pvm {

class Reader {

public:

	std::shared_ptr<Image> read(std::istream& stream) const;

private:

	static std::vector<uint8_t> read_raw_file(std::istream& stream);

	static constexpr size_t DDS_Blocksize = 1 << 20;

};

}}}
