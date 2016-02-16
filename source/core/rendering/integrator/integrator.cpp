#include "integrator.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/prop/prop_intersection.inl"
#include "take/take_settings.hpp"
#include "base/math/random/generator.inl"
#include "base/math/vector.inl"

namespace rendering { namespace integrator {

Integrator::Integrator(const take::Settings& settings, math::random::Generator& rng) :
	take_settings_(settings), rng_(rng) {}

Integrator::~Integrator() {}

void Integrator::start_new_pixel(uint32_t /*num_samples*/) {}

const take::Settings& Integrator::take_settings() const {
	return take_settings_;
}

}}
