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

#define SU_LOG_INFO 0
#define SU_LOG_WARNING 1
#define SU_LOG_ERROR 2
#define SU_LOG_VERBOSE 3

SU_LIBRARY_API char const* su_platform_revision() noexcept;

SU_LIBRARY_API int32_t su_init() noexcept;

SU_LIBRARY_API int32_t su_release() noexcept;

SU_LIBRARY_API int32_t su_mount(char const* folder) noexcept;

SU_LIBRARY_API int32_t su_load_take(char const* string) noexcept;

SU_LIBRARY_API int32_t su_create_defaults() noexcept;

SU_LIBRARY_API uint32_t su_create_camera(char const* string) noexcept;

SU_LIBRARY_API uint32_t su_create_camera_perspective(uint32_t width, uint32_t height, float fov) noexcept;

SU_LIBRARY_API int32_t su_create_sampler(uint32_t num_samples) noexcept;

SU_LIBRARY_API int32_t su_create_integrators(char const* string) noexcept;


SU_LIBRARY_API uint32_t su_create_image(uint32_t pixel_type, uint32_t num_channels, uint32_t width,
                                        uint32_t height, uint32_t depth, uint32_t num_elements,
                                        char const* data, uint32_t stride) noexcept;

SU_LIBRARY_API uint32_t su_create_material(char const* string) noexcept;

SU_LIBRARY_API uint32_t su_create_material_from_file(char const* filename) noexcept;

SU_LIBRARY_API uint32_t su_create_triangle_mesh(
    uint32_t num_vertices, float const* positions, uint32_t positions_stride, float const* normals,
    uint32_t normals_stride, float const* tangents, uint32_t tangents_stride,
    float const* texture_coordinates, uint32_t texture_coordinates_stride, uint32_t num_indices,
    uint32_t const* indices, uint32_t num_parts, uint32_t const* parts) noexcept;

SU_LIBRARY_API uint32_t su_create_triangle_mesh_from_file(char const* filename) noexcept;

SU_LIBRARY_API uint32_t su_create_prop(uint32_t shape, uint32_t num_materials,
                                       uint32_t const* materials) noexcept;

SU_LIBRARY_API int32_t su_create_light(uint32_t entity) noexcept;

SU_LIBRARY_API uint32_t su_camera_entity() noexcept;

SU_LIBRARY_API int32_t su_entity_set_transformation(uint32_t     entity,
                                                    float const* transformation) noexcept;

SU_LIBRARY_API int32_t su_render() noexcept;

SU_LIBRARY_API int32_t su_render_frame(uint32_t frame) noexcept;

typedef void (*Post)(uint32_t type, char const* text);

SU_LIBRARY_API int32_t su_register_log(Post post, bool verbose) noexcept;

typedef void (*Progress_start)(uint32_t resolution);

typedef void (*Progress_tick)();

SU_LIBRARY_API int32_t su_register_progress(Progress_start start, Progress_tick tick) noexcept;

}

#endif
