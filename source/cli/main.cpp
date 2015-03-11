#include "math/vector.inl"
#include <iostream>

int main() {
	std::cout << "Welcome to sprout!" << std::endl;

	math::float3 v(0.4, 0.2, 0.3);

	std::cout << "v: " << v.x << ", " << v.y << ", " << v.z << std::endl;

	return 0;
}
