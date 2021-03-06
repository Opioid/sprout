from ctypes import *
import platform
import math

LOG_FUNC = CFUNCTYPE(None, c_uint, c_char_p)

PROGRESS_START_FUNC = CFUNCTYPE(None, c_uint)

PROGRESS_TICK_FUNC = CFUNCTYPE(None)

def py_log_callback(msg_type, msg):
    if 1 == msg_type:
        print("Warning: " + str(msg, "utf-8"))
    elif 2 == msg_type:
        print("Error: " + str(msg, "utf-8"))
    else:
        print(str(msg, "utf-8"))

class Progressor:
    def start(self, resolution):
        self.resolution = resolution
        self.progress = 0
        self.threshold = 1.0

    def tick(self):
        if self.progress >= self.resolution:
            pass

        self.progress += 1

        p = float(self.progress) / float(self.resolution) * 100.0

        if p >= self.threshold:
            self.threshold += 1.0
            print("{}%".format(int(p)), end = "\r")

progress = Progressor()

def py_progress_start(resolution):
    global progress
    progress.start(resolution)

def py_progress_tick():
    global progress
    progress.tick()

if platform.system() == "Windows":
    sprout = CDLL("./sprout.dll")
else:
    sprout = CDLL("./libsprout.so")

#print(sprout.su_platform_revision().value)

logfunc = LOG_FUNC(py_log_callback)

progstartfunc = PROGRESS_START_FUNC(py_progress_start)
progtickfunc = PROGRESS_TICK_FUNC(py_progress_tick)

sprout.su_register_log(logfunc, False)

sprout.su_init(False)

sprout.su_register_progress(progstartfunc, progtickfunc)

sprout.su_mount(c_char_p(b"../../data/"))

#var = input("Please enter something: ")

#sprout.su_load_take(c_char_p(b"takes/cornell.take"))

resolution = (640, 360)

camera = sprout.su_create_camera_perspective(resolution[0], resolution[1], c_float(math.radians(70)))

# integrators_desc = """{
# "surface": {
# "AO": {}
# }
# }"""

integrators_desc = """{
"surface": {
"PTMIS": {}
}
}"""

sprout.su_create_integrators(c_char_p(integrators_desc.encode('utf-8')))

num_samples = 128;
sprout.su_create_sampler(num_samples)

sprout.su_create_defaults();

material_a_desc = """{
"rendering": {
    "Substitute": {
        "color": [0, 1, 0.5],
        "roughness": 0.2,
        "metallic": 0
    }
}
}"""

material_a = c_uint(sprout.su_create_material(c_char_p(material_a_desc.encode('utf-8'))));

Buffer = c_float * 12

image_buffer = Buffer(1.0, 0.0, 0.0,
                      0.0, 1.0, 0.0,
                      0.0, 0.0, 1.0,
                      1.0, 1.0, 0.0)

pixel_type = 2
num_channels = 3
width = 2
height = 2
depth = 1
num_elements = 1
stride = 12

image_a = sprout.su_create_image(pixel_type, num_channels, width, height, depth,
                                 num_elements, stride, image_buffer)

material_b_desc = """{{
"rendering": {{
    "Substitute": {{
        "textures": [
            {{
                "usage": "Color",
                "id": {}
            }}
        ],
        "color": [1, 1, 1],
        "roughness": 0.5,
        "metallic": 0
    }}
}}
}}""".format(image_a)

material_b = c_uint(sprout.su_create_material(c_char_p(material_b_desc.encode('utf-8'))));

material_light_desc = """{
"rendering": {
    "Light": {
        "emission": [10000, 10000, 10000]
    }
}
}"""

material_light = c_uint(sprout.su_create_material(c_char_p(material_light_desc.encode('utf-8'))));

sphere_a = sprout.su_create_prop(7, 1, byref(material_a))

plane_a = sprout.su_create_prop(5, 1, byref(material_b))

distant_sphere = sprout.su_create_prop(3, 1, byref(material_light))

sprout.su_create_light(distant_sphere)

Vertices = c_float * 9

positions = Vertices(-1.0, -1.0, 0.0,
                      0.0,  1.0, 0.0,
                      1.0, -1.0, 0.0)

normals = Vertices(0.0, 0.0, -1.0,
                   0.0, 0.0, -1.0,
                   0.0, 0.0, -1.0)

Tangents = c_float * 12

tangents = Tangents(1.0, 0.0, 0.0, 1.0,
                    1.0, 0.0, 0.0, 1.0,
                    1.0, 0.0, 0.0, 1.0)

UVs = c_float * 6

uvs = UVs(0.0, 1.0,
          0.5, 0.0,
          1.0, 1.0)

Indices = c_uint * 3

indices = Indices(0, 1, 2)

Parts = c_uint * 3
parts = Parts(0, 3, 0)

num_triangles = 1
num_vertices = 3
num_parts = 1

vertices_stride = 3
tangents_stride = 4
uvs_stride = 2

triangle = sprout.su_create_triangle_mesh_async(num_triangles, num_vertices,
                                                vertices_stride, positions,
                                                vertices_stride, normals,
                                                tangents_stride, tangents,
                                                uvs_stride, uvs,
                                                indices,
                                                num_parts, parts)

triangle_a = sprout.su_create_prop(triangle, 1, byref(material_a))

sprout.su_entity_allocate_frames(triangle_a)

Transformation = c_float * 16

transformation = Transformation(1.0, 0.0, 0.0, 0.0,
                                0.0, 1.0, 0.0, 0.0,
                                0.0, 0.0, 1.0, 0.0,
                                0.0, 1.0, 0.0, 1.0)

sprout.su_entity_set_transformation(camera, transformation)

transformation = Transformation(1.0, 0.0, 0.0, 0.0,
                                0.0, 1.0, 0.0, 0.0,
                                0.0, 0.0, 1.0, 0.0,
                                1.0, 1.0, 5.0, 1.0)

sprout.su_entity_set_transformation(sphere_a, transformation)

transformation = Transformation(1.0, 0.0, 0.0, 0.0,
                                0.0, 0.0, -1.0, 0.0,
                                0.0, 1.0, 0.0, 0.0,
                                0.0, 0.0, 0.0, 1.0)

sprout.su_entity_set_transformation(plane_a, transformation)

transformation = Transformation(0.01, 0.0, 0.0, 0.0,
                                0.0, 0.0, 0.01, 0.0,
                                0.0, -0.01, 0.0, 0.0,
                                0.0, 0.0, 0.0, 1.0)

sprout.su_entity_set_transformation(distant_sphere, transformation)

transformation = Transformation(1.0, 0.0, 0.0, 0.0,
                                0.0, 1.0, 0.0, 0.0,
                                0.0, 0.0, 1.0, 0.0,
                                -1.0, 1.0, 5.0, 1.0)

#sprout.su_entity_set_transformation(triangle_a, transformation)
sprout.su_entity_set_transformation_frame(triangle_a, 0, transformation)

transformation = Transformation(1.0, 0.0, 0.0, 0.0,
                                0.0, 1.0, 0.0, 0.0,
                                0.0, 0.0, 1.0, 0.0,
                                -1.0, 1.5, 5.0, 1.0)

sprout.su_entity_set_transformation_frame(triangle_a, 1, transformation)

sprout.su_render_frame(0)
sprout.su_export_frame(0)

sprout.su_entity_set_transformation_frame(triangle_a, 0, transformation)

transformation = Transformation(1.0, 0.0, 0.0, 0.0,
                                0.0, 1.0, 0.0, 0.0,
                                0.0, 0.0, 1.0, 0.0,
                                -1.0, 2.0, 5.0, 1.0)

sprout.su_entity_set_transformation_frame(triangle_a, 0, transformation)

sprout.su_render_frame(1)
sprout.su_export_frame(1)
