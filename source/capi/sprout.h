#ifndef SU_CAPI_SPROUT_H
#define SU_CAPI_SPROUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#if defined _WIN32
#ifdef SU_LIBRARY_EXPORTS
#define SU_LIBRARY_API __declspec(dllexport)
#else
#define SU_LIBRARY_API __declspec(dllimport)
#endif
#else
#define SU_LIBRARY_API __attribute__((visibility("default")))
#endif

#define SU_RESOURCE_NULL 0xFFFFFFFF

#define SU_SHAPE_CANOPY 0
#define SU_SHAPE_CUBE 1
#define SU_SHAPE_DISK 2
#define SU_SHAPE_DISTANT_SPHERE 3
#define SU_SHAPE_INFINITE_SPHERE 4
#define SU_SHAPE_PLANE 5
#define SU_SHAPE_RECTANGLE 6
#define SU_SHAPE_SPHERE 7

#define SU_UINT8 0
#define SU_FLOAT32 2

#define SU_LOG_INFO 0
#define SU_LOG_WARNING 1
#define SU_LOG_ERROR 2
#define SU_LOG_VERBOSE 3

SU_LIBRARY_API char const* su_platform_revision(void);

SU_LIBRARY_API int32_t su_init(bool progressive);

SU_LIBRARY_API int32_t su_release(void);

SU_LIBRARY_API int32_t su_mount(char const* folder);

SU_LIBRARY_API int32_t su_clear(void);

SU_LIBRARY_API int32_t su_load_take(char const* string);

SU_LIBRARY_API int32_t su_create_defaults(void);

SU_LIBRARY_API uint32_t su_create_camera(char const* string);

SU_LIBRARY_API uint32_t su_create_camera_perspective(uint32_t width, uint32_t height, float fov);

SU_LIBRARY_API int32_t su_camera_set_resolution(uint32_t width, uint32_t height);

SU_LIBRARY_API int32_t su_create_sampler(uint32_t num_samples);

SU_LIBRARY_API int32_t su_create_integrators(char const* string);

SU_LIBRARY_API uint32_t su_create_image(uint32_t pixel_type, uint32_t num_channels, uint32_t width,
                                        uint32_t height, uint32_t depth, uint32_t num_elements,
                                        uint32_t stride, char const* data);

SU_LIBRARY_API uint32_t su_create_material(char const* string);

SU_LIBRARY_API uint32_t su_create_material_from_file(char const* filename);

SU_LIBRARY_API uint32_t su_create_triangle_mesh_async(
    uint32_t num_triangles, uint32_t num_vertices, uint32_t positions_stride,
    float const* positions, uint32_t normals_stride, float const* normals, uint32_t tangents_stride,
    float const* tangents, uint32_t uvs_stride, float const* uvs, uint32_t const* indices,
    uint32_t num_parts, uint32_t const* parts);

SU_LIBRARY_API uint32_t su_create_triangle_mesh_from_file(char const* filename);

SU_LIBRARY_API uint32_t su_create_prop(uint32_t shape, uint32_t num_materials,
                                       uint32_t const* materials);

SU_LIBRARY_API int32_t su_create_light(uint32_t entity);

SU_LIBRARY_API uint32_t su_camera_entity();

SU_LIBRARY_API int32_t su_camera_sensor_dimensions(int32_t* dimensions);

SU_LIBRARY_API int32_t su_entity_allocate_frames(uint32_t entity);

SU_LIBRARY_API int32_t su_entity_transformation(uint32_t entity, float* trafo);

SU_LIBRARY_API int32_t su_entity_set_transformation(uint32_t entity, float const* trafo);

SU_LIBRARY_API int32_t su_entity_set_transformation_frame(uint32_t entity, uint32_t frame,
                                                          float const* trafo);

SU_LIBRARY_API int32_t su_render(void);

SU_LIBRARY_API int32_t su_render_frame(uint32_t frame);

SU_LIBRARY_API int32_t su_export_frame(uint32_t frame);

SU_LIBRARY_API int32_t su_start_render_frame(uint32_t frame);

SU_LIBRARY_API int32_t su_set_expected_iterations(uint32_t iterations);

SU_LIBRARY_API int32_t su_render_iteration(void);

SU_LIBRARY_API int32_t su_postprocess(void);

SU_LIBRARY_API int32_t su_copy_framebuffer(uint32_t type, uint32_t width, uint32_t height,
                                           uint32_t num_channels, uint8_t* destination);

typedef void (*Post)(uint32_t type, char const* text);

SU_LIBRARY_API int32_t su_register_log(Post post);

typedef void (*Progress_start)(uint32_t resolution);

typedef void (*Progress_tick)();

SU_LIBRARY_API int32_t su_register_progress(Progress_start start, Progress_tick tick);

#ifdef __cplusplus
}
#endif

#endif
