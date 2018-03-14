#include "log_std_out.hpp"
#include <iostream>

namespace logging {

void Std_out::post(Type type, std::string_view text) {
	switch (type) {
	case Type::Info:
	default:
		std::cout << text << std::endl;
		break;
	case Type::Warning:
		std::cout << "Warning: " << text << std::endl;
		break;
	case Type::Error:
		std::cout << "Error: " << text << std::endl;
		break;
	case Type::Verbose:
		std::cout << text << std::endl;
		break;
	case Type::Script_print:
		std::cout << "$: " << text << std::endl;
		break;
	case Type::Script_error:
		std::cout << "$!: " << text << std::endl;
		break;
	}
}

}
