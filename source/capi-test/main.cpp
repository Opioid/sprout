#include "capi/sprout.h"

#include <iostream>

int main(int /*argc*/, char* /*argv*/[]) noexcept {
	std::cout << "sprout capi test" << std::endl;

	su_init();

	su_load_take("takes/imrod.take");

	uint32_t const materials[] = {4};

	uint32_t const prop = su_create_prop(7, 1, materials);

	float const transformation[] = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
                                        0.f, 0.f, 1.f, 0.f, -1.f, 1.f, 0.f, 1.f};

	su_prop_set_transformation(prop, transformation);

	su_render();

	su_release();

	return 0;
}