#include "texture.hpp"

namespace image {

Texture::Texture(std::shared_ptr<Image> image) : image_(image) {}

inline Texture::operator bool() const {
	return image_.get() != nullptr;
}

inline bool Texture::is_valid() const {
	return image_.get() != nullptr;
}

const Image* Texture::image() const {
	return image_.get();
}

}


