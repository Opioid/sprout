#pragma once

#include <memory>

namespace image {

class Image;

class Texture {
public:

	Texture(std::shared_ptr<Image> image);

	explicit operator bool() const noexcept;

protected:

	std::shared_ptr<Image> image_;
};

}

