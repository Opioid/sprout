#ifndef SU_CORE_LOGGING_LOG_HPP
#define SU_CORE_LOGGING_LOG_HPP

#include <string>

namespace logging {

class Log {
  public:
    enum class Type { Info, Warning, Error, Verbose };

    virtual ~Log();

    virtual void post(Type type, std::string_view text) = 0;

    virtual void post(Type type, std::string const& text, std::string const& a);
};

}  // namespace logging

#endif
