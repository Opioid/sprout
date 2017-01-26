#include "sampler_settings.hpp"

namespace scene { namespace material {

Sampler_settings::Sampler_settings(Filter filter, Address address) :
	filter(filter), address(address) {}

uint32_t Sampler_settings::key() const {
	uint32_t key = static_cast<uint32_t>(filter) | static_cast<uint32_t>(address);
	return key;
}

}}
