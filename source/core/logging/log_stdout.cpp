#include "log_stdout.hpp"
#include <iostream>

namespace logging {

void Stdout::post(Type type, const std::string& text) {
	switch (type) {
	case Type::Warning:
		std::cout << "Warning: " << text << std::endl;
		break;
	case Type::Error:
		std::cout << "Error: " << text << std::endl;
		break;
	case Type::Info:
	default:
		std::cout << text << std::endl;
		break;
	}
}

}
