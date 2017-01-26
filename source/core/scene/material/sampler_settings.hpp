#pragma once

#include <cstdint>

namespace scene { namespace material {

struct Sampler_settings {

	enum class Address : uint32_t {
		Clamp	= 0 << 0,
		Repeat	= 1 << 0,
		Unknown = 0xFFFFFFFF
	};

	enum class Filter : uint32_t {
		Nearest = 0 << 1,
		Linear	= 1 << 1,
		Unknown = 0xFFFFFFFF
	};

	Sampler_settings(Filter filter = Filter::Linear, Address address = Address::Repeat);

	uint32_t key() const;

	Filter filter;
	Address address;
};

}}
