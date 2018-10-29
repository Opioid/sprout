#include "metal_presets.hpp"
#include "base/math/vector3.inl"

namespace scene::material::metal {

void ior_and_absorption(std::string const& name, float3& ior, float3& absorption) {
    // http://www.3delight.com/en/modules/forum/viewtopic.php?t=4205
    // R: 642nm G: 529nm B: 464nm
    // Table of wavelength-dependent values for ior and absorption (k)
    // Metal                 index (n)              extinction (k)
    // ---------  ----------------------------   ------------------------
    // Aluminium  (1.50694, 0.926041, 0.68251)   (7.6307, 6.3849, 5.6230)
    // Chromium   (3.11847, 3.02492, 2.44207)    (3.3190, 3.3322, 3.2034)
    // Cobalt     (2.86991, 2.19091, 1.80915)    (4.3419, 3.8804, 3.4920)
    // Copper     (0.23268, 1.075714, 1.15753)   (3.5315, 2.5945, 2.4473)
    // Gold       (0.18267, 0.49447, 1.3761)     (3.1178, 2.3515, 1.8324)
    // Iridium    (2.54731, 2.13062, 1.86627)    (4.6443, 4.2061, 3.7653)
    // Lithium    (0.22336, 0.20973, 0.23001)    (2.9727, 2.3544, 1.9843)
    // Molybdenu  (3.72055, 3.70022, 3.15441)    (3.5606, 3.6863, 3.5371)
    // Nickel     (1.99412, 1.73204, 1.64542)    (3.7682, 3.1316, 2.7371)
    // Osmium     (3.85401, 4.95607, 5.31627)    (1.6997, 1.8716, 3.2281)
    // Palladium  (1.78071, 1.59124, 1.44152)    (4.3345, 3.7268, 3.3428)
    // Platinum   (2.35423, 2.06875, 1.88085)    (4.1988, 3.6120, 3.2271)
    // Rhodium    (2.17407, 1.92204, 1.81356)    (5.6790, 4.8682, 4.5063)
    // Silver     (0.13708, 0.12945, 0.14075)    (4.0625, 3.1692, 2.6034)
    // Tantalum   (1.65178, 2.61195, 2.83282)    (2.1475, 1.8786, 2.0726)
    // Titanium   (2.18385, 1.8364,  1.72103)    (2.9550, 2.5188, 2.3034)
    // Tungsten   (3.66845, 3.48243, 3.31552)    (2.9274, 2.7200, 2.5721)
    // Vanadium   (3.51454, 3.66353, 3.23655)    (2.9684, 3.0917, 3.3679)
    // Iron       (2.9038,  2.8857,  2.6420)     (3.0798, 2.9157, 2.8025)

    if ("Aluminium" == name) {
        ior        = float3(1.50694f, 0.926041f, 0.68251f);
        absorption = float3(7.6307f, 6.3849f, 5.6230f);
    } else if ("Chromium" == name) {
        ior        = float3(3.11847f, 3.02492f, 2.44207f);
        absorption = float3(3.3190f, 3.3322f, 3.2034f);
    } else if ("Cobalt" == name) {
        ior        = float3(2.86991f, 2.19091f, 1.80915f);
        absorption = float3(4.3419f, 3.8804f, 3.4920f);
    } else if ("Copper" == name) {
        ior        = float3(0.23268f, 1.075714f, 1.15753f);
        absorption = float3(3.5315f, 2.5945f, 2.4473f);
    } else if ("Gold" == name) {
        ior        = float3(0.18267f, 0.49447f, 1.3761f);
        absorption = float3(3.1178f, 2.3515f, 1.8324f);
    } else if ("Iridium" == name) {
        ior        = float3(2.54731f, 2.13062f, 1.86627f);
        absorption = float3(4.6443f, 4.2061f, 3.7653f);
    } else if ("Lithium" == name) {
        ior        = float3(0.22336f, 0.20973f, 0.23001f);
        absorption = float3(2.9727f, 2.3544f, 1.9843f);
    } else if ("Molybdenu" == name) {
        ior        = float3(3.72055f, 3.70022f, 3.15441f);
        absorption = float3(3.5606f, 3.6863f, 3.5371f);
    } else if ("Nickel" == name) {
        ior        = float3(1.99412f, 1.73204f, 1.64542f);
        absorption = float3(3.7682f, 3.1316f, 2.7371f);
    } else if ("Osmium" == name) {
        ior        = float3(3.85401f, 4.95607f, 5.31627f);
        absorption = float3(1.6997f, 1.8716f, 3.2281f);
    } else if ("Palladium" == name) {
        ior        = float3(1.78071f, 1.59124f, 1.44152f);
        absorption = float3(4.3345f, 3.7268f, 3.3428f);
    } else if ("Platinum" == name) {
        ior        = float3(2.35423f, 2.06875f, 1.88085f);
        absorption = float3(4.1988f, 3.6120f, 3.2271f);
    } else if ("Rhodium" == name) {
        ior        = float3(2.17407f, 1.92204f, 1.81356f);
        absorption = float3(5.6790f, 4.8682f, 4.5063f);
    } else if ("Silver" == name) {
        ior        = float3(0.13708f, 0.12945f, 0.14075f);
        absorption = float3(4.0625f, 3.1692f, 2.6034f);
    } else if ("Tantalum" == name) {
        ior        = float3(1.65178f, 2.61195f, 2.83282f);
        absorption = float3(2.1475f, 1.8786f, 2.0726f);
    } else if ("Titanium" == name) {
        ior        = float3(2.18385f, 1.8364f, 1.72103f);
        absorption = float3(2.9550f, 2.5188f, 2.3034f);
    } else if ("Tungsten" == name) {
        ior        = float3(3.66845f, 3.48243f, 3.31552f);
        absorption = float3(2.9274f, 2.7200f, 2.5721f);
    } else if ("Vanadium" == name) {
        ior        = float3(3.51454f, 3.66353f, 3.23655f);
        absorption = float3(2.9684f, 3.0917f, 3.3679f);
    } else if ("Iron" == name) {
        ior        = float3(2.9038f, 2.8857f, 2.6420f);
        absorption = float3(3.0798f, 2.9157f, 2.8025f);
    }
}

}  // namespace scene::material::metal
