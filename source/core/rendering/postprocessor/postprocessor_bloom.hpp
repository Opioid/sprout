#pragma once

#include "postprocessor.hpp"
#include <vector>

namespace rendering { namespace postprocessor {

class Bloom : public Postprocessor {

public:

	Bloom(float angle, float alpha, float threshold, float intensity);

	virtual void init(const scene::camera::Camera& camera, thread::Pool& pool) override final;

	virtual size_t num_bytes() const override final;

private:

	virtual void apply(int32_t begin, int32_t end, uint32_t pass,
					   const image::Float_4& source,
					   image::Float_4& destination) override final;

	float angle_;
	float alpha_;
	float threshold_;
	float intensity_;

	image::Float_4 scratch_;

	struct K {
		int32_t o;
		float w;
	};

	std::vector<K> kernel_;
};

}}
