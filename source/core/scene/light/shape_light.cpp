#include "shape_light.hpp"
#include "light_sample.hpp"
#include "scene/shape/shape.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

namespace scene { namespace light {

void Shape_light::init(std::shared_ptr<shape::Shape> shape) {
	shape_ = shape;
}

void Shape_light::sample(const math::float3& p, float time, uint32_t max_samples, sampler::Sampler& sampler,
						 std::vector<Sample>& samples) const {
	Composed_transformation transformation;
	transformation_at(time, transformation);

	samples.clear();

	Sample sample;

	shape_->importance_sample(transformation, p, sampler, sample.l, sample.t, sample.pdf);

	sample.energy = lumen_ * color_;

	samples.push_back(sample);
}

void Shape_light::set_color(const math::float3& color) {
	color_ = color;
}

void Shape_light::set_lumen(float lumen) {
	lumen_ = lumen;
}

}}
