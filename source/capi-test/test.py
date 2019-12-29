from ctypes import *

LOG_FUNC = CFUNCTYPE(None, c_uint, c_char_p)

PROGRESS_START_FUNC = CFUNCTYPE(None, c_uint)

PROGRESS_TICK_FUNC = CFUNCTYPE(None)

def py_log_callback(msg_type, msg):
    print(msg)

def py_progress_start(resolution):
    print("start")

def py_progress_tick():
    print("tick")

#sprout = CDLL("./sprout.dll")
sprout = CDLL("./libsprout.so")

#print(sprout.su_platform_revision().value)

logfunc = LOG_FUNC(py_log_callback)

progstartfunc = PROGRESS_START_FUNC(py_progress_start)
progtickfunc = PROGRESS_TICK_FUNC(py_progress_tick)

sprout.su_register_log(logfunc, False)
sprout.su_register_progress(progstartfunc, progtickfunc)

sprout.su_init()

sprout.su_mount(create_string_buffer(b"../../data/"))

#var = input("Please enter something: ")

sprout.su_load_take(create_string_buffer(b"takes/cornell.take"))

sprout.su_render()
