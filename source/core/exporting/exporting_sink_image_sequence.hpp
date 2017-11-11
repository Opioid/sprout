#ifndef SU_CORE_EXPORTING_SINK_IMAGE_SEQUENCE_HPP
#define SU_CORE_EXPORTING_SINK_IMAGE_SEQUENCE_HPP

#include "exporting_sink.hpp"
#include <memory>
#include <string>

namespace image { class Writer; }

namespace exporting {

class Image_sequence : public Sink {

public:

	Image_sequence(const std::string& filename, std::unique_ptr<image::Writer> writer);

	virtual void write(const image::Float4& image, uint32_t frame,
					   thread::Pool& pool) override final;

private:

	std::string filename_;

	std::unique_ptr<image::Writer> writer_;
};

}

#endif
