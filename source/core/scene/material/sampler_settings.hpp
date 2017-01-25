#pragma once

#include <cstdint>

namespace scene { namespace material {

struct Sampler_settings {

	enum class Filter {
		Nearest,
		Linear,
		Unknown
	};

	enum class Address {
		Clamp,
		Repeat,
		Unknown
	};

	Sampler_settings(Filter filter = Filter::Linear, Address address = Address::Repeat);

	uint32_t key() const;

	Filter filter;
	Address address;
};

}}
