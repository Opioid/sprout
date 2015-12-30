#include "log_stdout.hpp"
#include <iostream>

namespace logging {

void Stdout::post(Type /*type*/, const std::string& text) {
	std::cout << text << std::endl;
}

}

