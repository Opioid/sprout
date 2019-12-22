#ifndef SU_CAPI_SPROUT_H
#define SU_CAPI_SPROUT_H

#include "visibility.h"

#include <cstdint>

extern "C" {

	SU_LIBRARY_API char const* su_platform_revision() noexcept;

	SU_LIBRARY_API int32_t su_init() noexcept;

	SU_LIBRARY_API int32_t su_release() noexcept;

	SU_LIBRARY_API int32_t su_load_take(char *const string) noexcept;

	SU_LIBRARY_API uint32_t su_create_prop(uint32_t shape, uint32_t num_materials,
                                           uint32_t const* materials) noexcept;

	SU_LIBRARY_API int32_t su_create_light(uint32_t prop) noexcept;

	SU_LIBRARY_API int32_t su_prop_set_transformation(uint32_t prop, float const* transformation) noexcept;

	SU_LIBRARY_API int32_t su_render() noexcept;

	SU_LIBRARY_API int square(int i) noexcept;
}

#endif
