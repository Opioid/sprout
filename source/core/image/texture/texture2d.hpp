#pragma once

#include "texture.hpp"
#include "base/math/vector.hpp"

namespace image {

class Texture2D : public Texture {
public:

	Texture2D(std::shared_ptr<Image> image);

	math::float3 at3(uint32_t x, uint32_t y) const;
	math::float4 at4(uint32_t x, uint32_t y) const;
};

}
