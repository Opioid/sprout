#ifndef SU_CAPI_SPROUT_H
#define SU_CAPI_SPROUT_H

#include "visibility.h"

#include <cstdint>

extern "C" {

	SU_LIBRARY_API char const* su_platform_revision(void) noexcept;

	SU_LIBRARY_API void su_init(void) noexcept;

	SU_LIBRARY_API void su_release(void) noexcept;

	SU_LIBRARY_API int32_t su_load_take(char *const string) noexcept;

	SU_LIBRARY_API int32_t su_render() noexcept;

	SU_LIBRARY_API int square(int i) noexcept;
}

#endif
