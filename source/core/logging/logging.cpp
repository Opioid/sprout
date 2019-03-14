#include "logging.hpp"
#include "base/memory/unique.inl"
#include "log_null.hpp"
#include "log_std_out.hpp"

namespace logging {

static memory::Unique_ptr<Log> log;

static bool log_verbose = false;

void init(Type type, bool verbose) {
    switch (type) {
        case Type::Null:
            log = memory::Unique_ptr<Log>(new Null);
            break;
        case Type::Std_out:
        default:
            log = memory::Unique_ptr<Log>(new Std_out);
            break;
    }

    log_verbose = verbose;
}

void info(std::string const& text) {
    log->post(Log::Type::Info, text);
}

void warning(std::string const& text) {
    log->post(Log::Type::Warning, text);
}

void warning(std::string const& text, std::string const& a) {
    log->post(Log::Type::Warning, text, a);
}

void error(std::string const& text) {
    log->post(Log::Type::Error, text);
}

void error(std::string const& text, std::string const& a) {
    log->post(Log::Type::Error, text, a);
}

void push_error(std::string const& text) {
    log->push(Log::Type::Error, text);
}

void push_error(std::string const& text, std::string const& a) {
    log->push(Log::Type::Error, text, a);
}

void verbose(std::string const& text) {
    if (log_verbose) {
        log->post(Log::Type::Verbose, text);
    }
}

void verbose(std::string const& text, std::string const& a) {
    if (log_verbose) {
        log->post(Log::Type::Verbose, text, a);
    }
}

void set_verbose(bool verbose) {
    log_verbose = verbose;
}

bool is_verbose() {
    return log_verbose;
}

}  // namespace logging
