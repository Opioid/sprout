#include "logging.hpp"
#include "log_null.hpp"
#include "log_std_out.hpp"

namespace logging {

Log* log = nullptr;

void init(Type type) {
	switch (type) {
	case Type::Null:
		log = new Null;
		break;
	case Type::Std_out:
	default:
		log = new Std_out;
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
