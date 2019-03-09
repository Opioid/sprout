#ifndef SU_CORE_LOGGING_STD_OUT_HPP
#define SU_CORE_LOGGING_STD_OUT_HPP

#include "log.hpp"

namespace logging {

class Std_out : public Log {
  private:
    void internal_post(Type type, std::string const& text) override final;
};

}  // namespace logging

#endif
