#include "texture2d.hpp"
#include "image/image.hpp"

namespace image {

Texture2D::Texture2D(std::shared_ptr<Image> image) : Texture(image) {}

math::float3 Texture2D::at3(uint32_t x, uint32_t y) const {
	uint32_t i = y * image_->description().dimensions.x + x;
	return image_->at3(i);
}

math::float4 Texture2D::at4(uint32_t x, uint32_t y) const {
	uint32_t i = y * image_->description().dimensions.x + x;
	return image_->at4(i);
}

}
