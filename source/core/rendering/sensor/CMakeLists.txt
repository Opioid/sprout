add_subdirectory(filter)

target_sources(core
	PRIVATE
	"clamp.hpp"
	"clamp.inl"
	"filtered.hpp"
	"filtered.inl"
	"opaque.cpp"
	"opaque.hpp"
	"sensor.cpp"
	"sensor.hpp"
	"transparent.cpp"
	"transparent.hpp"
	"unfiltered.hpp"
	"unfiltered.inl"
	) 
