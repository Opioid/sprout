#pragma once

namespace scene { namespace material {

struct Sampler_settings {

	enum class Filter {
		Nearest,
		Linear,
		Unknown
	};

	Sampler_settings(Filter filter = Filter::Linear);

	Filter filter;
};

}}
