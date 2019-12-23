#include "capi/sprout.h"

#include <iostream>

int main(int /*argc*/, char* /*argv*/[]) noexcept {
	std::cout << "sprout capi test" << std::endl;

	su_init();

	su_load_take("takes/imrod.take");

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

	float const transformation[] = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
                                        0.f, 0.f, 1.f, 0.f, -1.f, 1.f, 0.f, 1.f};

	su_entity_set_transformation(prop, transformation);

	su_render();

	su_release();

	return 0;
}