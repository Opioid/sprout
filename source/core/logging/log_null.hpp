#ifndef SU_CORE_LOGGING_NULL_HPP
#define SU_CORE_LOGGING_NULL_HPP

#include "log.hpp"

namespace logging {

class Null : public Log {
  public:
    void post(Type type, std::string const& text) override final;

    void post(Type type, std::string const& text, float a) override final;

    void post(Type type, std::string const& text, std::string const& a) override final;

    void push(Type type, std::string const& text) override final;

    void push(Type type, std::string const& text, std::string const& a) override final;

  private:
    void internal_post(Type type, std::string const& text) override final;
};

}  // namespace logging

#endif
