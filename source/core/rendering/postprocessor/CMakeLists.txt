add_subdirectory(tonemapping)

target_sources(core
	PRIVATE
	"postprocessor_backplate.cpp"
	"postprocessor_backplate.hpp"
	"postprocessor_glare.cpp"
	"postprocessor_glare.hpp"
	"postprocessor_pipeline.cpp"
	"postprocessor_pipeline.hpp"
	"postprocessor.cpp"
	"postprocessor.hpp"
	) 
