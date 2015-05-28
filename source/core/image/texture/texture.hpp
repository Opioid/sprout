#pragma once

#include <memory>

namespace image {

class Image;

class Texture {
public:

	Texture(std::shared_ptr<Image> image);

	void init(std::shared_ptr<Image> image);

	explicit operator bool() const;

	bool is_valid() const;

	const Image* image() const;

protected:

	std::shared_ptr<Image> image_;
};

}

