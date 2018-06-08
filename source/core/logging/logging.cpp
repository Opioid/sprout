#include "logging.hpp"
#include <memory>
#include "log_null.hpp"
#include "log_std_out.hpp"

namespace logging {

static std::unique_ptr<Log> log;
static bool log_verbose = false;

void init(Type type, bool verbose) {
  switch (type) {
    case Type::Null:
      log = std::make_unique<Null>();
      break;
    case Type::Std_out:
    default:
      log = std::make_unique<Std_out>();
      break;
  }

  log_verbose = verbose;
}

void info(std::string_view text) { log->post(Log::Type::Info, text); }

void warning(std::string_view text) { log->post(Log::Type::Warning, text); }

void error(std::string_view text) { log->post(Log::Type::Error, text); }

void verbose(std::string_view text) {
  if (log_verbose) {
    log->post(Log::Type::Verbose, text);
  }
}

void set_verbose(bool verbose) { log_verbose = verbose; }

bool is_verbose() { return log_verbose; }

}  // namespace logging
