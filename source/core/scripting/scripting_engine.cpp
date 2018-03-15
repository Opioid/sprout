#include "scripting_engine.hpp"
#include "scripting_print.hpp"
#include "logging/logging.hpp"

namespace scripting {

void error_handler(void* user_data, const char* message);

Engine::Engine() : context_(duk_create_heap(nullptr, nullptr, nullptr, nullptr, error_handler)) {
	duk_print_init(context_);
}

Engine::~Engine() {
	duk_destroy_heap(context_);
}

void Engine::eval(const std::string& text) const {
	if (0 != duk_peval_string(context_, text.data())) {
		logging::error("scripting::eval(): " + std::string(duk_safe_to_string(context_, -1)));
	}
}

duk_context* Engine::context() const {
	return context_;
}

void error_handler(void* /*user_data*/, const char* message) {
	logging::error(message);
}

}
