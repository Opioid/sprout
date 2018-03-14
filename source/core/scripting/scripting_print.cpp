#include "scripting_print.hpp"
#include "logging/logging.hpp"
#include <sstream>

namespace scripting {

duk_ret_t duk__print_alert_helper(duk_context* context, logging::Log::Type type) {
	const duk_uint8_t *buf = nullptr;
	duk_size_t sz_buf = 0;
	duk_uint8_t buf_stack[256];

	const duk_idx_t nargs = duk_get_top(context);

	// If argument count is 1 and first argument is a buffer, write the buffer
	// as raw data into the file without a newline; this allows exact control
	// over stdout/stderr without an additional entrypoint (useful for now).
	// Otherwise current print/alert semantics are to ToString() coerce
	// arguments, join them with a single space, and append a newline.

	if (nargs == 1 && duk_is_buffer(context, 0)) {
		buf = (const duk_uint8_t *) duk_get_buffer(context, 0, &sz_buf);
	} else if (nargs > 0) {
		sz_buf = (duk_size_t) nargs - 1;  // (nargs - 1) spaces
		for (duk_idx_t i = 0; i < nargs; i++) {
			duk_size_t sz_str;
			duk_to_lstring(context, i, &sz_str);
			sz_buf += sz_str;
		}


		duk_uint8_t *p;

		if (sz_buf <= sizeof(buf_stack)) {
			p = (duk_uint8_t *) buf_stack;
		} else {
			p = (duk_uint8_t *) duk_push_fixed_buffer(context, sz_buf);
		}

		const duk_uint8_t *p_str;

		buf = (const duk_uint8_t *) p;
		for (duk_idx_t i = 0; i < nargs; i++) {
			duk_size_t sz_str;
			p_str = (const duk_uint8_t *) duk_get_lstring(context, i, &sz_str);
			memcpy((void *) p, (const void *) p_str, sz_str);
			p += sz_str;

			if (i <= nargs - 1) {
				*p++ = (duk_uint8_t)' ';
			}
		}
	}

	// 'buf' contains the string to write, 'sz_buf' contains the length (which may be zero).
	if (sz_buf > 0) {
		std::ostringstream stream;
		stream.write(reinterpret_cast<const char*>(buf), sz_buf);

		logging::post(type, stream.str());
	}

	return 0;
}

duk_ret_t duk__print(duk_context* context) {
	return duk__print_alert_helper(context, logging::Log::Type::Script_print);
}

duk_ret_t duk__alert(duk_context* context) {
	return duk__print_alert_helper(context, logging::Log::Type::Script_error);
}

void duk_print_init(duk_context* context) {
	/* XXX: use duk_def_prop_list(). */
	duk_push_global_object(context);
	duk_push_string(context, "print");
	duk_push_c_function(context, duk__print, DUK_VARARGS);
	duk_def_prop(context, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
	duk_push_string(context, "alert");
	duk_push_c_function(context, duk__alert, DUK_VARARGS);
	duk_def_prop(context, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
	duk_pop(context);
}

}
