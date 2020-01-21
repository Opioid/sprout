from ctypes import *
import platform
import math

import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

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

sprout.su_init(True)

sprout.su_register_progress(progstartfunc, progtickfunc)

sprout.su_mount(c_char_p(b"../../data/"))

sprout.su_load_take(c_char_p(b"takes/cornell.take"))

resolution = (720, 720)

Image = ((c_uint8 * 3) * resolution[0]) * resolution[1]

image = Image()

dpi = 100

fig = plt.figure(figsize=(resolution[0]/dpi, resolution[1]/dpi), dpi=dpi)
im = fig.figimage(image)

label = plt.figtext(0.0, 0.0, "0", color=(1.0, 1.0, 0.0))

sprout.su_start_render_frame(0)

def init():
    pass

def update(frame_number):
    sprout.su_render_iteration(frame_number)

    sprout.su_copy_framebuffer(0, resolution[0], resolution[1], 3, image)

    im.set_data(image)

    label.set_text(str(frame_number))

animation = FuncAnimation(fig, update, init_func=init, interval=1)

plt.show()


