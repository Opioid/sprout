#pragma once

#include "film.hpp"

namespace rendering { namespace film {

class Opaque : public Film {
public:

	Opaque(math::uint2 dimensions, float exposure, std::unique_ptr<tonemapping::Tonemapper> tonemapper);
	virtual ~Opaque();

	virtual void clear() final override;

protected:

	virtual void add_pixel(uint32_t x, uint32_t y, const math::float4& color, float weight) final override;

	virtual void add_pixel_atomic(uint32_t x, uint32_t y, const math::float4& color, float weight) final override;

	virtual void resolve(uint32_t begin, uint32_t end) final override;

	struct Pixel {
		math::float3 color;
		float        weight_sum;
	};

	Pixel* pixels_;
};


}}
