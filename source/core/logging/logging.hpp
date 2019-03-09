#ifndef SU_CORE_LOGGING_LOGGING_HPP
#define SU_CORE_LOGGING_LOGGING_HPP

#include <string>

namespace logging {

enum class Type { Null, Std_out };

void init(Type type, bool verbose = false);

void info(std::string const& text);

void warning(std::string const& text);
void warning(std::string const& text, std::string const& a);

void error(std::string const& text);
void error(std::string const& text, std::string const& a);

void push_error(std::string const& text);
void push_error(std::string const& text, std::string const& a);

void verbose(std::string const& text);

void set_verbose(bool verbose);
bool is_verbose();

}  // namespace logging

#endif
