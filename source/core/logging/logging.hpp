#ifndef SU_CORE_LOGGING_LOGGING_HPP
#define SU_CORE_LOGGING_LOGGING_HPP

#include <string>

namespace logging {

enum class Type {
	Null,
	Std_out
};

void init(Type type, bool verbose = false);
void release();

void info(const std::string& text);
void warning(const std::string& text);
void error(const std::string& text);
void verbose(const std::string& text);

void set_verbose(bool verbose);
bool is_verbose();

}

#endif
