#pragma once

namespace image { namespace texture {

class Texture {
public:

	enum class Type {
		_2D,
	};

	Texture(Type type);
	virtual ~Texture();

protected:

	Type type_;
};

}}

