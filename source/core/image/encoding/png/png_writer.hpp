#ifndef SU_CORE_IMAGE_ENCODING_PNG_WRITER_HPP
#define SU_CORE_IMAGE_ENCODING_PNG_WRITER_HPP

#include "image/image_writer.hpp"
#include "image/encoding/encoding_srgb.hpp"

namespace image::encoding::png {

class Writer : public image::Writer, Srgb {

public:

	Writer(int2 dimensions);

	virtual std::string file_extension() const override final;

	virtual bool write(std::ostream& stream, Float4 const& image,
					   thread::Pool& pool) override final;

	static bool write(std::string const& name, Byte3 const& image);

	static bool write(std::string const& name, Byte1 const& image);

	static bool write(std::string const& name, float const* data, int2 dimensions, float scale);

	static bool write(std::string const& name, float2 const* data, int2 dimensions, float scale);
};

class Writer_alpha : public image::Writer, Srgb_alpha {

public:

	Writer_alpha(int2 dimensions);

	virtual std::string file_extension() const override final;

	virtual bool write(std::ostream& stream, Float4 const& image,
					   thread::Pool& pool) override final;
};

}

#endif
