#ifndef SU_CORE_LOGGING_NULL_HPP
#define SU_CORE_LOGGING_NULL_HPP

#include "log.hpp"

namespace logging {

class Null : public Log {
  public:
    virtual void post(Type type, std::string_view text) override final;
};

}  // namespace logging

#endif
