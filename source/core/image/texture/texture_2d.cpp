#include "texture_2d.hpp"
#include "image/image.hpp"

namespace image {

Texture_2D::Texture_2D(std::shared_ptr<Image> image) : Texture(image) {}

math::uint2 Texture_2D::dimensions() const {
	return image_->description().dimensions;
}

float Texture_2D::at1(uint32_t x, uint32_t y) const {
	uint32_t i = y * image_->description().dimensions.x + x;
	return image_->at1(i);
}

math::float3 Texture_2D::at3(uint32_t x, uint32_t y) const {
	uint32_t i = y * image_->description().dimensions.x + x;
	return image_->at3(i);
}

math::float4 Texture_2D::at4(uint32_t x, uint32_t y) const {
	uint32_t i = y * image_->description().dimensions.x + x;
	return image_->at4(i);
}

}
