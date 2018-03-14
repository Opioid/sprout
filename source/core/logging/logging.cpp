#include "logging.hpp"
#include "log_null.hpp"
#include "log_std_out.hpp"
#include <memory>

namespace logging {

std::unique_ptr<Log> log;
bool log_verbose = false;

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

void post(Log::Type type, const std::string& text) {
	log->post(type, text);
}

void info(const std::string& text) {
	log->post(Log::Type::Info, text);
}

void warning(const std::string& text) {
	log->post(Log::Type::Warning, text);
}

void error(const std::string& text) {
	log->post(Log::Type::Error, text);
}

void verbose(const std::string& text) {
	if (log_verbose) {
		log->post(Log::Type::Verbose, text);
	}
}

void set_verbose(bool verbose) {
	log_verbose = verbose;
}

bool is_verbose() {
	return log_verbose;
}

}
