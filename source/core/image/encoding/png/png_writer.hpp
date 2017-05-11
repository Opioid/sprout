#pragma once

#include "image/image_writer.hpp"
#include "image/encoding/encoding_srgb.hpp"

namespace image { namespace encoding { namespace png {

class Writer : public image::Writer, Srgb_alpha {

public:

	Writer(int2 dimensions);

	virtual std::string file_extension() const final override;

	bool write(std::ostream& stream, const Float_3& image, thread::Pool& pool);

	virtual bool write(std::ostream& stream, const Float_4& image,
					   thread::Pool& pool) final override;

	static bool write(const std::string& name, const Byte_3& image);

	static bool write(const std::string& name, const Byte_1& image);

	static bool write(const std::string& name, const float* data, int2 dimensions, float scale);

	static bool write(const std::string& name, const float2* data, int2 dimensions, float scale);
};

}}}
