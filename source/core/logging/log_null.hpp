#ifndef SU_CORE_LOGGING_NULL_HPP
#define SU_CORE_LOGGING_NULL_HPP

#include "log.hpp"

namespace logging {

class Null : public Log {
  public:
    void post(Type type, std::string_view text) override final;

    void post(Type type, std::string const& text, std::string const& a) override final;
};

}  // namespace logging

#endif
