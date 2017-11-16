#ifndef SU_EXTENSION_PROCEDURAL_AURORA_VOLUME_RASTERIZER_HPP
#define SU_EXTENSION_PROCEDURAL_AURORA_VOLUME_RASTERIZER_HPP

#include "image/typed_image.hpp"

namespace procedural::aurora {

class Volume_rasterizer {

public:

	Volume_rasterizer(const int3& dimensions);
	~Volume_rasterizer();

	void resolve(image::Byte3& target) const;

	void set_brush(const float3& color);

	void clear();

	void splat(const float3& position, const float3& color);

	void draw_sphere(const float3& pos, float radius);

private:

	int3 dimensions_;

	int32_t area_;

	float3 brush_;

	float3* samples_;
};

}

#endif
