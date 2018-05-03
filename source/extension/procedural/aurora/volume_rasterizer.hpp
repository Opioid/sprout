#ifndef SU_EXTENSION_PROCEDURAL_AURORA_VOLUME_RASTERIZER_HPP
#define SU_EXTENSION_PROCEDURAL_AURORA_VOLUME_RASTERIZER_HPP

#include "image/typed_image_fwd.hpp"
#include "base/math/vector3.hpp"

namespace procedural::aurora {

class Volume_rasterizer {

public:

	Volume_rasterizer(const int3& dimensions);
	~Volume_rasterizer();

	void resolve(image::Byte3& target) const;

	void set_brush(float3 const& color);

	void clear();

	void splat(f_float3 position, float3 const& color);

	float3* data();

private:

	int3 dimensions_;

	int32_t area_;

	float3 brush_;

	float3* samples_;
};

}

#endif
