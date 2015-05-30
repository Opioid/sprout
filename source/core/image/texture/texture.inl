#pragma once

#include "texture.hpp"

namespace image {

inline Texture::Texture(std::shared_ptr<Image> image) : image_(image) {}

inline void Texture::init(std::shared_ptr<Image> image) {
	image_ = image;
}

inline Texture::operator bool() const {
	return image_.get() != nullptr;
}

inline bool Texture::is_valid() const {
	return image_.get() != nullptr;
}

inline const Image* Texture::image() const {
	return image_.get();
}

}


