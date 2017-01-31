#pragma once

#include <cstdint>

namespace scene { namespace material {

struct Sampler_settings {

	enum class Address : uint32_t {
		Clamp	= 0 << 0,
		Repeat	= 1 << 0,
		Unknown = 0xFFFFFFFF
	};

	enum class Address_flat : uint32_t {
		Clamp_clamp,
		Clamp_repeat,
		Repeat_clamp,
		Repeat_repeat,
		Mask = 0x00000003
	};

	enum class Filter : uint32_t {
		Nearest = 0 << 2,
		Linear	= 1 << 2,
		Unknown = 0xFFFFFFFF
	};

	Sampler_settings(Filter filter = Filter::Linear,
					 Address address_u = Address::Repeat,
					 Address address_v = Address::Repeat);

	uint32_t key() const;

	Filter filter;
	Address address_u;
	Address address_v;
};

}}
