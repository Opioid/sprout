#include "log_std_out.hpp"
#include <iostream>

namespace logging {

void Std_out::internal_post(Type type, std::string const& text) {
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
    }
}

}  // namespace logging
