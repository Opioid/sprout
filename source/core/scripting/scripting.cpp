#include "scripting.hpp"
#include "scripting_print.hpp"
#include "logging/logging.hpp"
#include "base/string/string.hpp"

#include "duktape/duktape.h"

namespace scripting {

void error_handler(void* user_data, const char* message);

duk_context* context;

void init() {
	context = duk_create_heap(nullptr, nullptr, nullptr, nullptr, error_handler);

	duk_print_init(context);
}

void close() {
	duk_destroy_heap(context);
}

void test() {
//	duk_eval_string(context, "print(1+2);print('stuff');print({hello;hullo;})");

	duk_peval_string(context, "var thing = {hello: 'name',hullo: 'age'};print(thing.hello)");

//	duk_int_t result = duk_get_int(context, -1);

//	logging::info(string::to_string(result));

//	printf("1+2=%d\n", (int) duk_get_int(ctx, -1));
}

void error_handler(void* /*user_data*/, const char* message) {
	logging::error(message);
}

}
