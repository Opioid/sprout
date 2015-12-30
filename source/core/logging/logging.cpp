#include "logging.hpp"
#include "log_null.hpp"
#include "log_stdout.hpp"


namespace logging {

Log* log = nullptr;

void init(Type type) {
	switch (type) {
	case Type::Null:
		log = new Null;
		break;
	case Type::Stdout:
	default:
		log = new Stdout;
		break;
	}
}

void release() {
	delete log;
	log = nullptr;
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

}
