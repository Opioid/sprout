#include "sub_pathtracer.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace integrator { namespace surface { namespace sub {

Pathtracer::Pathtracer(const take::Settings& settings, rnd::Generator& rng) :
	Integrator(settings, rng) {}

void Pathtracer::prepare(const Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Pathtracer::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Pathtracer::li(Worker& worker, const Ray& ray, const Intersection& intersection) {
	return float3(1.f, 0.f, 0.f);
}

size_t Pathtracer::num_bytes() const {
	return sizeof(*this);
}

}}}}
