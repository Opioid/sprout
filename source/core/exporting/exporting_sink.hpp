#pragma once

namespace image {

class Image;

}

namespace thread {

class Pool;

}

namespace exporting {

class Sink {
public:

	virtual ~Sink() {}

	virtual void write(const image::Image& image, thread::Pool& pool) = 0;
};

}
