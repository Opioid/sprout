import numpy as np

def xy_to_XYZ(xy):
    return np.array([xy[0] / xy[1], 1, (1 - xy[0] - xy[1]) / xy[1]])

# Formula from: http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
def rgb_to_xyz(r, g, b, w):
    xyz_r = xy_to_XYZ(r)
    xyz_g = xy_to_XYZ(g)
    xyz_b = xy_to_XYZ(b)
    xyz_w = xy_to_XYZ(w)

    s = xyz_w * np.matrix([xyz_r, xyz_g, xyz_b]).getI()

    m = np.matrix([[s[0, 0] * xyz_r[0], s[0, 1] * xyz_g[0], s[0, 2] * xyz_b[0]],
                   [s[0, 0] * xyz_r[1], s[0, 1] * xyz_g[1], s[0, 2] * xyz_b[1]],
                   [s[0, 0] * xyz_r[2], s[0, 1] * xyz_g[2], s[0, 2] * xyz_b[2]]])

    return m

# http://www.brucelindbloom.com/index.html?Eqn_ChromAdapt.html
# https://github.com/ampas/aces-dev/blob/master/transforms/ctl/lib/ACESlib.Utilities_Color.ctl

def cat(ws, wd):
    b = np.matrix([[0.8951000, -0.7502000, 0.0389000],
                   [0.2664000, 1.7135000, -0.0685000],
                   [-0.1614000, 0.0367000, 1.0296000]])

    xyz_ws = xy_to_XYZ(ws)
    xyz_wd = xy_to_XYZ(wd)

    pyb_s = xyz_ws * b
    pyb_d = xyz_wd * b

    mid = np.diag((pyb_d / pyb_s).A1)

    return (b * mid * b.getI()).transpose()

# https://en.wikipedia.org/wiki/Academy_Color_Encoding_System#Converting_CIE_XYZ_values_to_ACES2065-1_values
ap1_r = [0.713, 0.293]
ap1_g = [0.165, 0.830]
ap1_b = [0.128, 0.044]
aces_w = [0.32168, 0.33767]

ap1_to_xyz = rgb_to_xyz(ap1_r, ap1_g, ap1_b, aces_w)
xyz_to_ap1 = ap1_to_xyz.getI()

# print(ap1_to_xyz)
# print("XYZ to AP1")
# print(xyz_to_ap1)

# http://terathon.com/blog/rgb-xyz-conversion-matrix-accuracy/
srgb_r = [0.64, 0.33]
srgb_g = [0.3, 0.6]
srgb_b = [0.15, 0.06]
d65 = [0.3127, 0.3290]

srgb_to_xyz = rgb_to_xyz(srgb_r, srgb_g, srgb_b, d65)
xyz_to_srgb = srgb_to_xyz.getI()

print("sRGB to XYZ to AP1")
print(xyz_to_ap1 * srgb_to_xyz)

d60_to_d65 = np.matrix([[0.987224, -0.00611327, 0.0159533],
                        [-0.00759836, 1.00186, 0.00533002],
                        [0.00307257, -0.00509595, 1.08168]])

d65_to_d60 = d60_to_d65.getI()

# print("sRGB to XYZ to d65 to d60 to AP1")
# print(xyz_to_ap1 * d65_to_d60 * srgb_to_xyz)

# print((xyz_to_ap1 * d65_to_d60 * srgb_to_xyz).getI())


# print("my bradford")
# print(cat(aces_w, d65))

# print("aces bradford")
# print(d60_to_d65)


# white_cat = cat(d65, aces_w)

# print(xyz_to_ap1 * white_cat)
