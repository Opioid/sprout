#pragma once

#include "texture_2d.hpp"
#include "texture.inl"
#include "image/image.hpp"

namespace image {

inline Texture_2D::Texture_2D(std::shared_ptr<Image> image) : Texture(image) {}

inline math::uint2 Texture_2D::dimensions() const {
	return image_->description().dimensions;
}

inline float Texture_2D::at1(uint32_t x, uint32_t y) const {
	uint32_t i = y * image_->description().dimensions.x + x;
	return image_->at1(i);
}

inline math::float2 Texture_2D::at2(uint32_t x, uint32_t y) const {
	uint32_t i = y * image_->description().dimensions.x + x;
	return image_->at2(i);
}

inline math::float3 Texture_2D::at3(uint32_t x, uint32_t y) const {
	uint32_t i = y * image_->description().dimensions.x + x;
	return image_->at3(i);
}

inline math::float4 Texture_2D::at4(uint32_t x, uint32_t y) const {
	uint32_t i = y * image_->description().dimensions.x + x;
	return image_->at4(i);
}

}
