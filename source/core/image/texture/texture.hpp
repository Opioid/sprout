#pragma once

#include <memory>

namespace image {

class Image;

class Texture {
public:

	Texture(std::shared_ptr<Image> image);

protected:


	std::shared_ptr<Image> image_;
};

}

