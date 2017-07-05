#pragma once

#include "image/image_writer.hpp"
#include "image/encoding/encoding_srgb.hpp"

namespace image { namespace encoding { namespace png {

class Writer : public image::Writer, Srgb_alpha {

public:

	Writer(int2 dimensions);

	virtual std::string file_extension() const override final;

	bool write(std::ostream& stream, const Float3& image, thread::Pool& pool);

	virtual bool write(std::ostream& stream, const Float4& image,
					   thread::Pool& pool) override final;

	static bool write(const std::string& name, const Byte3& image);

	static bool write(const std::string& name, const Byte1& image);

	static bool write(const std::string& name, const float* data, int2 dimensions, float scale);

	static bool write(const std::string& name, const float2* data, int2 dimensions, float scale);
};

}}}
