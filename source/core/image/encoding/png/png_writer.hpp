#pragma once

#include "image/image_writer.hpp"
#include "image/encoding/encoding_srgb.hpp"

namespace image { namespace encoding { namespace png {

class Writer : public image::Writer, Srgb_alpha {

public:

	Writer(int2 dimensions);

	virtual std::string file_extension() const final override;

	bool write(std::ostream& stream, const Image_float_3& image, thread::Pool& pool);

	virtual bool write(std::ostream& stream, const Image_float_4& image,
					   thread::Pool& pool) final override;

	static bool write(const std::string& name, const Image_byte_3& image);

	static bool write(const std::string& name, const Image_byte_1& image);
};

}}}
