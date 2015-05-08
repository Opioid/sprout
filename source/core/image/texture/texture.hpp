#pragma once

#include <memory>

namespace image {

class Image;

class Texture {
public:

	Texture(std::shared_ptr<Image> image);

	explicit operator bool() const;

	bool is_valid() const;

protected:

	std::shared_ptr<Image> image_;
};

}

