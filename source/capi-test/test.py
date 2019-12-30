from ctypes import *
import platform
import math

LOG_FUNC = CFUNCTYPE(None, c_uint, c_char_p)

PROGRESS_START_FUNC = CFUNCTYPE(None, c_uint)

PROGRESS_TICK_FUNC = CFUNCTYPE(None)

def py_log_callback(msg_type, msg):
    print(msg)

def py_progress_start(resolution):
    print("start")

def py_progress_tick():
    pass
   # print("tick")

if platform.system() == "Windows":
    sprout = CDLL("./sprout.dll")
else:
    sprout = CDLL("./libsprout.so")

#print(sprout.su_platform_revision().value)

logfunc = LOG_FUNC(py_log_callback)

progstartfunc = PROGRESS_START_FUNC(py_progress_start)
progtickfunc = PROGRESS_TICK_FUNC(py_progress_tick)

sprout.su_register_log(logfunc, False)
sprout.su_register_progress(progstartfunc, progtickfunc)

sprout.su_init()

sprout.su_mount(c_char_p(b"../../data/"))

#var = input("Please enter something: ")

#sprout.su_load_take(c_char_p(b"takes/cornell.take"))

camera = sprout.su_create_camera_perspective(640, 360, c_float(math.radians(70)))

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

image_a = sprout.su_create_image(2, 3, 2, 2, 1, 1, image_buffer, 12)

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

celestial_disk = sprout.su_create_prop(1, 1, byref(material_light))

sprout.su_create_light(celestial_disk)

Transformation = c_float * 16

transformation = Transformation(1.0, 0.0, 0.0, 0.0,
                                0.0, 1.0, 0.0, 0.0,
                                0.0, 0.0, 1.0, 0.0,
                                0.0, 1.0, 0.0, 1.0)

sprout.su_entity_set_transformation(camera, transformation)

transformation = Transformation(1.0, 0.0, 0.0, 0.0,
                                0.0, 1.0, 0.0, 0.0,
                                0.0, 0.0, 1.0, 0.0,
                                0.0, 1.0, 5.0, 1.0)

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

sprout.su_entity_set_transformation(celestial_disk, transformation)

sprout.su_render()
