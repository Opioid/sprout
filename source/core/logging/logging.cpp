#include "logging.hpp"
#include "base/memory/unique.inl"
#include "log.hpp"

namespace logging {

static memory::Unique_ptr<Log> log;

void init(Log* l) noexcept {
    log = memory::Unique_ptr<Log>(l);
}

void info(std::string const& text) noexcept {
    if (log) {
        log->post(Log::Type::Info, text);
    }
}

void info(std::string const& text, float a) noexcept {
    if (log) {
        log->post(Log::Type::Info, text, a);
    }
}

void info(std::string const& text, std::string const& a) noexcept {
    if (log) {
        log->post(Log::Type::Info, text, a);
    }
}

void warning(std::string const& text) noexcept {
    if (log) {
        log->post(Log::Type::Warning, text);
    }
}

void warning(std::string const& text, std::string const& a) noexcept {
    if (log) {
        log->post(Log::Type::Warning, text, a);
    }
}

void error(std::string const& text) noexcept {
    if (log) {
        log->post(Log::Type::Error, text);
    }
}

void error(std::string const& text, std::string const& a) noexcept {
    if (log) {
        log->post(Log::Type::Error, text, a);
    }
}

void push_error(std::string const& text) noexcept {
    if (log) {
        log->push(Log::Type::Error, text);
    }
}

void push_error(std::string const& text, std::string const& a) noexcept {
    if (log) {
        log->push(Log::Type::Error, text, a);
    }
}

#ifdef SU_DEBUG

void verbose(std::string const& text) noexcept {
    if (log) {
        log->post(Log::Type::Verbose, text);
    }
}

void verbose(std::string const& text, std::string const& a) noexcept {
    if (log) {
        log->post(Log::Type::Verbose, text, a);
    }
}

#endif

}  // namespace logging
