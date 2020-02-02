#include "capi/sprout.h"
#include "core/progress/progress_sink_std_out.hpp"

void logging_post(uint32_t type, char const* text);

void progress_start(uint32_t resolution);
void progress_tick();

int main(int /*argc*/, char* /*argv*/[]) {
    std::cout << "sprout capi test" << std::endl;

    su_init(false);

    su_register_log(&logging_post);

    su_register_progress(&progress_start, &progress_tick);

    if (su_load_take("takes/imrod.take")) {
        return 1;
    }

    std::string const material_source = R"({
	"rendering": {
		"Substitute": {
			"color": [0, 1, 0.5],
			"roughness": 0.0,
			"metallic": 1
		}
	}
})";

    uint32_t const material = su_create_material(material_source.c_str());

    uint32_t const materials[] = {material};

    uint32_t const prop = su_create_prop(SU_SHAPE_SPHERE, 1, materials);

    float const transformation[] = {1.f, 0.f, 0.f, 0.f, 0.f,  1.f, 0.f, 0.f,
                                    0.f, 0.f, 1.f, 0.f, -1.f, 1.f, 0.f, 1.f};

    su_entity_set_transformation(prop, transformation);

    su_render();

    su_release();

    return 0;
}

void logging_post(uint32_t type, char const* text) {
    if (SU_LOG_WARNING == type) {
        std::cout << "Warning: ";
    } else if (SU_LOG_ERROR == type) {
        std::cout << "Error: ";
    }

    std::cout << text << std::endl;
}

progress::Std_out progressor;

void progress_start(uint32_t resolution) {
    progressor.start(resolution);
}

void progress_tick() {
    progressor.tick();
}
