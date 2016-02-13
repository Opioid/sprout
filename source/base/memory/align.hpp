#pragma once

namespace memory {

#ifdef __GNUG__
#	define ALIGN(X) __attribute__ ((aligned(X)))
#elif defined(_MSC_VER)
#	define ALIGN(X) __declspec(align(X))
#else
#	error Unknown compiler, unknown alignment attribute!
#endif

}
