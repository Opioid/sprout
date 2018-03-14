#ifndef SU_CORE_LOGGING_LOGGING_HPP
#define SU_CORE_LOGGING_LOGGING_HPP

#include "log.hpp"
#include <string>

namespace logging {

enum class Type {
	Null,
	Std_out
};

void init(Type type, bool verbose = false);

void post(Log::Type type, std::string_view text);
void info(std::string_view text);
void warning(std::string_view text);
void error(std::string_view text);
void verbose(std::string_view text);

void set_verbose(bool verbose);
bool is_verbose();

}

#endif
