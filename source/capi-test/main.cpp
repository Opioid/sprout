#include "capi/sprout.h"

#include <iostream>

int main(int /*argc*/, char* /*argv*/[]) noexcept {
	std::cout << "sprout capi test" << std::endl;

	su_init();

	su_load_take("takes/imrod.take");

	su_render();

	su_release();

	return 0;
}