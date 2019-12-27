#ifndef SU_CORE_LOGGING_STD_OUT_HPP
#define SU_CORE_LOGGING_STD_OUT_HPP

#include "log.hpp"

#include <iostream>

namespace logging {

class Std_out : public Log {
  private:
    void internal_post(Type type, std::string const& text) noexcept override final {
        switch (type) {
            case Type::Info:
            case Type::Verbose:
            default:
                break;
            case Type::Warning:
                std::cout << "Warning: ";
                break;
            case Type::Error:
                std::cout << "Error: ";
                break;
        }

        std::cout << text << std::endl;
    }
};

}  // namespace logging

#endif
