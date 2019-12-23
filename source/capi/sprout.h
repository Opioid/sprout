#ifndef SU_CAPI_SPROUT_H
#define SU_CAPI_SPROUT_H

#include "visibility.h"

#include <cstdint>

extern "C" {

#define SU_SHAPE_CANOPY 0
#define SU_SHAPE_CELESTIAL_DISK 1
#define SU_SHAPE_CUBE 2
#define SU_SHAPE_DISK 3
#define SU_SHAPE_INFINITE_SPHERE 4
#define SU_SHAPE_PLANE 5
#define SU_SHAPE_RECTANGLE 6
#define SU_SHAPE_SPHERE 7

SU_LIBRARY_API char const* su_platform_revision() noexcept;

SU_LIBRARY_API int32_t su_init() noexcept;

SU_LIBRARY_API int32_t su_release() noexcept;

SU_LIBRARY_API int32_t su_load_take(char const* string) noexcept;

SU_LIBRARY_API uint32_t su_create_camera(char const* string) noexcept;

SU_LIBRARY_API int32_t su_create_integrators(char const* string) noexcept;

SU_LIBRARY_API uint32_t su_create_material(char const* string) noexcept;

SU_LIBRARY_API uint32_t su_create_material_from_file(char const* filename) noexcept;

SU_LIBRARY_API uint32_t su_create_triangle_mesh_from_file(char const* filename) noexcept;

SU_LIBRARY_API uint32_t su_create_prop(uint32_t shape, uint32_t num_materials,
                                       uint32_t const* materials) noexcept;

SU_LIBRARY_API int32_t su_create_light(uint32_t entity) noexcept;

SU_LIBRARY_API uint32_t su_camera_entity() noexcept;

SU_LIBRARY_API int32_t su_entity_set_transformation(uint32_t     entity,
                                                    float const* transformation) noexcept;

SU_LIBRARY_API int32_t su_render() noexcept;
}

#endif
