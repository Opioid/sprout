#ifndef SU_CORE_LOGGING_LOGGING_HPP
#define SU_CORE_LOGGING_LOGGING_HPP

#include <string>

namespace logging {

class Log;

void init(Log* l);

void info(std::string const& text);
void info(std::string const& text, float a);
void info(std::string const& text, std::string const& a);

void warning(std::string const& text);
void warning(std::string const& text, std::string const& a);

void error(std::string const& text);
void error(std::string const& text, std::string const& a);

void push_error(std::string const& text);
void push_error(std::string const& text, std::string const& a);

#ifdef SU_DEBUG

void verbose(std::string const& text);
void verbose(std::string const& text, std::string const& a);

#endif

}  // namespace logging

#define GET_LOGGING_MACRO(_1, _2, NAME, ...) NAME
#define LOGGING_VERBOSE(...) \
    GET_LOGGING_MACRO(__VA_ARGS__, LOGGING_VERBOSE2, LOGGING_VERBOSE1)(__VA_ARGS__)

#ifndef SU_DEBUG

#define LOGGING_VERBOSE1(TEXT) ((void)0)
#define LOGGING_VERBOSE2(TEXT, A) ((void)0)

#else

#define LOGGING_VERBOSE1(TEXT) logging::verbose(TEXT)
#define LOGGING_VERBOSE2(TEXT, A) logging::verbose(TEXT, A)

#endif

#endif
