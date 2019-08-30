add_subdirectory(photon)

target_sources(core
    PRIVATE
	"lighttracer.cpp"
	"lighttracer.hpp"
    "particle_importance.cpp"
    "particle_importance.hpp"
	)
