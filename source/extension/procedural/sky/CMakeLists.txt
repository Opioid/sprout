add_subdirectory(hosek)

target_sources(extension
	PRIVATE
	"sky_material_base.cpp"
	"sky_material_base.hpp"
	"sky_material.cpp"
	"sky_material.hpp"
	"sky_model.cpp"
	"sky_model.hpp"
	"sky_provider.cpp"
	"sky_provider.hpp"
	"sky.cpp"
	"sky.hpp"
	"sun_material.cpp"
	"sun_material.hpp"
	)
