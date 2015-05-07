#include "texture.hpp"

namespace image {

Texture::Texture(std::shared_ptr<Image> image) : image_(image) {}

Texture::operator bool() const noexcept {
	return image_.get() != nullptr;
}

}


