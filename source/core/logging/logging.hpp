#ifndef SU_CORE_LOGGING_LOGGING_HPP
#define SU_CORE_LOGGING_LOGGING_HPP

#include <string>

namespace logging {

enum class Type { Null, Std_out };

void init(Type type, bool verbose = false) noexcept;

void info(std::string const& text) noexcept;
void info(std::string const& text, std::string const& a) noexcept;

void warning(std::string const& text) noexcept;
void warning(std::string const& text, std::string const& a) noexcept;

void error(std::string const& text) noexcept;
void error(std::string const& text, std::string const& a) noexcept;

void push_error(std::string const& text) noexcept;
void push_error(std::string const& text, std::string const& a) noexcept;

void verbose(std::string const& text) noexcept;
void verbose(std::string const& text, std::string const& a) noexcept;

void set_verbose(bool verbose) noexcept;
bool is_verbose() noexcept;

}  // namespace logging

#endif
