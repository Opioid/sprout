/*
This source is published under the following 3-clause BSD license.

Copyright (c) 2012 - 2013, Lukas Hosek and Alexander Wilkie
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * None of the names of the contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* ============================================================================

This file is part of a sample implementation of the analytical skylight and
solar radiance models presented in the SIGGRAPH 2012 paper


           "An Analytic Model for Full Spectral Sky-Dome Radiance"

and the 2013 IEEE CG&A paper

       "Adding a Solar Radiance Function to the Hosek Skylight Model"

                                   both by

                       Lukas Hosek and Alexander Wilkie
                Charles University in Prague, Czech Republic


                        Version: 1.4a, February 22nd, 2013

Version history:

1.4a  February 22nd, 2013
      Removed unnecessary and counter-intuitive solar radius parameters
      from the interface of the colourspace sky dome initialisation functions.

1.4   February 11th, 2013
      Fixed a bug which caused the relative brightness of the solar disc
      and the sky dome to be off by a factor of about 6. The sun was too
      bright: this affected both normal and alien sun scenarios. The
      coefficients of the solar radiance function were changed to fix this.

1.3   January 21st, 2013 (not released to the public)
      Added support for solar discs that are not exactly the same size as
      the terrestrial sun. Also added support for suns with a different
      radiance spectrum ("Alien World" functionality).

1.2a  December 18th, 2012
      Fixed a mistake and some inaccuracies in the solar radiance function
      explanations found in ArHosekSkyModel.h. The actual source code is
      unchanged compared to version 1.2.

1.2   December 17th, 2012
      Native RGB data and a solar radiance function that matches the turbidity
      conditions were added.

1.1   September 2012
      The coefficients of the spectral model are now scaled so that the output
      is given in physical units: W / (m^-2 * sr * nm). Also, the output of the
      XYZ model is now no longer scaled to the range [0...1]. Instead, it is
      the result of a simple conversion from spectral data via the CIE 2 degree
      standard observer matching functions. Therefore, after multiplication
      with 683 lm / W, the Y channel now corresponds to luminance in lm.

1.0   May 11th, 2012
      Initial release.


Please visit http://cgg.mff.cuni.cz/projects/SkylightModelling/ to check if
an updated version of this code has been published!

============================================================================ */

/*

All instructions on how to use this code are in the accompanying header file.

*/

#include "ArHosekSkyModel.hpp"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include "ArHosekSkyModelData_CIEXYZ.hpp"
#include "ArHosekSkyModelData_RGB.hpp"
#include "ArHosekSkyModelData_Spectral.hpp"

//   Some macro definitions that occur elsewhere in ART, and that have to be
//   replicated to make this a stand-alone module.

#ifndef NIL
#define NIL 0
#endif

#ifndef MATH_PI
#define MATH_PI 3.141592653589793
#endif

#ifndef MATH_DEG_TO_RAD
#define MATH_DEG_TO_RAD (MATH_PI / 180.0)
#endif

#ifndef MATH_RAD_TO_DEG
#define MATH_RAD_TO_DEG (180.0 / MATH_PI)
#endif

#ifndef DEGREES
#define DEGREES *MATH_DEG_TO_RAD
#endif

#ifndef TERRESTRIAL_SOLAR_RADIUS
#define TERRESTRIAL_SOLAR_RADIUS ((0.51 DEGREES) / 2.0)
#endif

#ifndef ALLOC
#define ALLOC(_struct) ((_struct*)malloc(sizeof(_struct)))
#endif

// internal definitions

typedef const hk_real* ArHosekSkyModel_Dataset;
typedef const hk_real* ArHosekSkyModel_Radiance_Dataset;

// internal functions

void ArHosekSkyModel_CookConfiguration(ArHosekSkyModel_Dataset      dataset,
                                       ArHosekSkyModelConfiguration config, hk_real turbidity,
                                       hk_real albedo, hk_real solar_elevation) {
    const hk_real* elev_matrix;

    int     int_turbidity = (int)turbidity;
    hk_real turbidity_rem = turbidity - (hk_real)int_turbidity;

    hk_real se  = std::pow(solar_elevation / (hk_real(MATH_PI) / hk_real(2)),
                          (hk_real(1) / hk_real(3)));
    hk_real se2 = se * se;
    hk_real se3 = se2 * se;
    hk_real se4 = se3 * se;
    hk_real se5 = se4 * se;

    hk_real omse  = hk_real(1) - se;
    hk_real omse2 = omse * omse;
    hk_real omse3 = omse2 * omse;
    hk_real omse4 = omse3 * omse;
    hk_real omse5 = omse4 * omse;

    hk_real a = hk_real(5) * omse4 * se;
    hk_real b = hk_real(10) * omse3 * se2;
    hk_real c = hk_real(10) * omse2 * se3;
    hk_real d = hk_real(5) * omse * se4;

    // alb 0 low turb

    elev_matrix = dataset + (9 * 6 * (int_turbidity - 1));

    for (unsigned int i = 0; i < 9; ++i) {
        //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
        config[i] = (hk_real(1) - albedo) * (hk_real(1) - turbidity_rem) *
                    (omse5 * elev_matrix[i] + a * elev_matrix[i + 9] + b * elev_matrix[i + 18] +
                     c * elev_matrix[i + 27] + d * elev_matrix[i + 36] + se5 * elev_matrix[i + 45]);
    }

    // alb 1 low turb
    elev_matrix = dataset + (9 * 6 * 10 + 9 * 6 * (int_turbidity - 1));
    for (unsigned int i = 0; i < 9; ++i) {
        //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
        config[i] += (albedo) * (hk_real(1) - turbidity_rem) *
                     (omse5 * elev_matrix[i] + a * elev_matrix[i + 9] + b * elev_matrix[i + 18] +
                      c * elev_matrix[i + 27] + d * elev_matrix[i + 36] +
                      se5 * elev_matrix[i + 45]);
    }

    if (int_turbidity == 10) return;

    // alb 0 high turb
    elev_matrix = dataset + (9 * 6 * (int_turbidity));
    for (unsigned int i = 0; i < 9; ++i) {
        //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
        config[i] += (hk_real(1) - albedo) * (turbidity_rem) *
                     (omse5 * elev_matrix[i] + a * elev_matrix[i + 9] + b * elev_matrix[i + 18] +
                      c * elev_matrix[i + 27] + d * elev_matrix[i + 36] +
                      se5 * elev_matrix[i + 45]);
    }

    // alb 1 high turb
    elev_matrix = dataset + (9 * 6 * 10 + 9 * 6 * (int_turbidity));
    for (unsigned int i = 0; i < 9; ++i) {
        //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
        config[i] += (albedo) * (turbidity_rem) *
                     (omse5 * elev_matrix[i] + a * elev_matrix[i + 9] + b * elev_matrix[i + 18] +
                      c * elev_matrix[i + 27] + d * elev_matrix[i + 36] +
                      se5 * elev_matrix[i + 45]);
    }
}

hk_real ArHosekSkyModel_CookRadianceConfiguration(ArHosekSkyModel_Radiance_Dataset dataset,
                                                  hk_real turbidity, hk_real albedo,
                                                  hk_real solar_elevation) {
    const hk_real* elev_matrix;

    int     int_turbidity = (int)turbidity;
    hk_real turbidity_rem = turbidity - (hk_real)int_turbidity;
    hk_real res;

    hk_real se  = std::pow(solar_elevation / (hk_real(MATH_PI) / hk_real(2)),
                          (hk_real(1) / hk_real(3)));
    hk_real se2 = se * se;
    hk_real se3 = se2 * se;
    hk_real se4 = se3 * se;
    hk_real se5 = se4 * se;

    hk_real omse  = hk_real(1) - se;
    hk_real omse2 = omse * omse;
    hk_real omse3 = omse2 * omse;
    hk_real omse4 = omse3 * omse;
    hk_real omse5 = omse4 * omse;

    hk_real a = hk_real(5) * omse4 * se;
    hk_real b = hk_real(10) * omse3 * se2;
    hk_real c = hk_real(10) * omse2 * se3;
    hk_real d = hk_real(5) * omse * se4;

    // alb 0 low turb
    elev_matrix = dataset + (6 * (int_turbidity - 1));
    //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
    res = (hk_real(1) - albedo) * (hk_real(1) - turbidity_rem) *
          (omse5 * elev_matrix[0] + a * elev_matrix[1] + b * elev_matrix[2] + c * elev_matrix[3] +
           d * elev_matrix[4] + se5 * elev_matrix[5]);

    // alb 1 low turb
    elev_matrix = dataset + (6 * 10 + 6 * (int_turbidity - 1));
    //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
    res += (albedo) * (hk_real(1) - turbidity_rem) *
           (omse5 * elev_matrix[0] + a * elev_matrix[1] + b * elev_matrix[2] + c * elev_matrix[3] +
            d * elev_matrix[4] + se5 * elev_matrix[5]);
    if (int_turbidity == 10) return res;

    // alb 0 high turb
    elev_matrix = dataset + (6 * (int_turbidity));
    //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
    res += (hk_real(1) - albedo) * (turbidity_rem) *
           (omse5 * elev_matrix[0] + a * elev_matrix[1] + b * elev_matrix[2] + c * elev_matrix[3] +
            d * elev_matrix[4] + se5 * elev_matrix[5]);

    // alb 1 high turb
    elev_matrix = dataset + (6 * 10 + 6 * (int_turbidity));
    //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
    res += (albedo) * (turbidity_rem) *
           (omse5 * elev_matrix[0] + a * elev_matrix[1] + b * elev_matrix[2] + c * elev_matrix[3] +
            d * elev_matrix[4] + se5 * elev_matrix[5]);
    return res;
}

hk_real pow1_5(hk_real x) {
    return x * std::sqrt(x);
}

hk_real ArHosekSkyModel_GetRadianceInternal(ArHosekSkyModelConfiguration configuration,
                                            //	hk_real                      theta,
                                            hk_real cos_theta, hk_real sqrt_cos_theta,
                                            hk_real gamma, hk_real cos_gamma) {
    const hk_real expM = std::exp(configuration[4] * gamma);
    //	const hk_real cos_gamma = std::cos(gamma);
    const hk_real rayM = cos_gamma * cos_gamma;
    //	const hk_real mieM = (hk_real(1.0) + std::cos(gamma)*std::cos(gamma)) /
    // std::pow((hk_real(1.0) + configuration[8]*configuration[8] -
    // hk_real(2.0)*configuration[8]*std::cos(gamma)), hk_real(1.5));
    const hk_real mieM = (hk_real(1.0) + rayM) /
                         pow1_5((hk_real(1.0) + configuration[8] * configuration[8] -
                                 hk_real(2.0) * configuration[8] * cos_gamma));
    //	const hk_real cos_theta = std::cos(theta);
    const hk_real zenith = sqrt_cos_theta;  // std::sqrt(cos_theta);

    return (hk_real(1.0) +
            configuration[0] * std::exp(configuration[1] / (cos_theta + hk_real(0.01)))) *
           (configuration[2] + configuration[3] * expM + configuration[5] * rayM +
            configuration[6] * mieM + configuration[7] * zenith);
}

// spectral version

ArHosekSkyModelState* arhosekskymodelstate_alloc_init(const hk_real solar_elevation,
                                                      const hk_real atmospheric_turbidity,
                                                      const hk_real ground_albedo) {
    ArHosekSkyModelState* state = ALLOC(ArHosekSkyModelState);

    //	state->solar_radius = (hk_real(0.51 DEGREES)) / hk_real(2);
    const hk_real solar_radius = (hk_real(0.51 DEGREES)) / hk_real(2);
    const hk_real sol_rad_sin  = std::sin(solar_radius);
    state->ar2                 = hk_real(1) / (sol_rad_sin * sol_rad_sin);
    state->turbidity           = atmospheric_turbidity;
    state->albedo              = ground_albedo;
    state->elevation           = solar_elevation;

    for (unsigned int wl = 0; wl < 11; ++wl) {
        ArHosekSkyModel_CookConfiguration(datasets[wl], state->configs[wl], atmospheric_turbidity,
                                          ground_albedo, solar_elevation);

        state->radiances[wl] = ArHosekSkyModel_CookRadianceConfiguration(
            datasetsRad[wl], atmospheric_turbidity, ground_albedo, solar_elevation);

        state->emission_correction_factor_sun[wl] = hk_real(1);
        state->emission_correction_factor_sky[wl] = hk_real(1);
    }

    return state;
}

//   'blackbody_scaling_factor'
//
//   Fudge factor, computed in Mathematica, to scale the results of the
//   following function to match the solar radiance spectrum used in the
//   original simulation. The scaling is done so their integrals over the
//   range from 380.0 to 720.0 nanometers match for a blackbody temperature
//   of 5800 K.
//   Which leaves the original spectrum being less bright overall than the 5.8k
//   blackbody radiation curve if the ultra-violet part of the spectrum is
//   also considered. But the visible brightness should be very similar.

const hk_real blackbody_scaling_factor = hk_real(3.19992 * 10E-11);

//   'art_blackbody_dd_value()' function
//
//   Blackbody radiance, Planck's formula

hk_real art_blackbody_dd_value(const hk_real temperature, const hk_real lambda) {
    hk_real c1 = hk_real(3.74177 * 10E-17);
    hk_real c2 = hk_real(0.0143878);
    hk_real value;

    value = (c1 / (std::pow(lambda, hk_real(5)))) *
            (hk_real(1) / (std::exp(c2 / (lambda * temperature)) - hk_real(1)));

    return value;
}

//   'originalSolarRadianceTable[]'
//
//   The solar spectrum incident at the top of the atmosphere, as it was used
//   in the brute force path tracer that generated the reference results the
//   model was fitted to. We need this as the yardstick to compare any altered
//   Blackbody radiance spectra for alien world stars to.

//   This is just the data from the Preetham paper, extended into the UV range.

const hk_real originalSolarRadianceTable[] = {hk_real(7500.0),  hk_real(12500.0), hk_real(21127.5),
                                              hk_real(26760.5), hk_real(30663.7), hk_real(27825.0),
                                              hk_real(25503.8), hk_real(25134.2), hk_real(23212.1),
                                              hk_real(21526.7), hk_real(19870.8)};

ArHosekSkyModelState* arhosekskymodelstate_alienworld_alloc_init(
    const hk_real solar_elevation, const hk_real solar_intensity,
    const hk_real solar_surface_temperature_kelvin, const hk_real atmospheric_turbidity,
    const hk_real ground_albedo) {
    ArHosekSkyModelState* state = ALLOC(ArHosekSkyModelState);

    state->turbidity = atmospheric_turbidity;
    state->albedo    = ground_albedo;
    state->elevation = solar_elevation;

    for (unsigned int wl = 0; wl < 11; ++wl) {
        //   Basic init as for the normal scenario

        ArHosekSkyModel_CookConfiguration(datasets[wl], state->configs[wl], atmospheric_turbidity,
                                          ground_albedo, solar_elevation);

        state->radiances[wl] = ArHosekSkyModel_CookRadianceConfiguration(
            datasetsRad[wl], atmospheric_turbidity, ground_albedo, solar_elevation);

        //   The wavelength of this band in nanometers

        hk_real owl = (hk_real(320) + hk_real(40) * wl) * hk_real(10E-10);

        //   The original intensity we just computed

        hk_real osr = originalSolarRadianceTable[wl];

        //   The intensity of a blackbody with the desired temperature
        //   The fudge factor described above is used to make sure the BB
        //   function matches the used radiance data reasonably well
        //   in magnitude.

        hk_real nsr = art_blackbody_dd_value(solar_surface_temperature_kelvin, owl) *
                      blackbody_scaling_factor;

        //   Correction factor for this waveband is simply the ratio of
        //   the two.

        state->emission_correction_factor_sun[wl] = nsr / osr;
    }

    //   We then compute the average correction factor of all wavebands.

    //   Theoretically, some weighting to favour wavelengths human vision is
    //   more sensitive to could be introduced here - think V(lambda). But
    //   given that the whole effort is not *that* accurate to begin with (we
    //   are talking about the appearance of alien worlds, after all), simple
    //   averaging over the visible wavelenghts (! - this is why we start at
    //   WL #2, and only use 2-11) seems like a sane first approximation.

    hk_real correctionFactor = hk_real(0);

    for (unsigned int i = 2; i < 11; i++) {
        correctionFactor += state->emission_correction_factor_sun[i];
    }

    //   This is the average ratio in emitted energy between our sun, and an
    //   equally large sun with the blackbody spectrum we requested.

    //   Division by 9 because we only used 9 of the 11 wavelengths for this
    //   (see above).

    hk_real ratio = correctionFactor / hk_real(9);

    //   This ratio is then used to determine the radius of the alien sun
    //   on the sky dome. The additional factor 'solar_intensity' can be used
    //   to make the alien sun brighter or dimmer compared to our sun.

    //    state->solar_radius =
    //		  ( std::sqrt( solar_intensity ) * hk_real(TERRESTRIAL_SOLAR_RADIUS))
    //		/ std::sqrt( ratio );

    const hk_real solar_radius = (std::sqrt(solar_intensity) * hk_real(TERRESTRIAL_SOLAR_RADIUS)) /
                                 std::sqrt(ratio);
    const hk_real sol_rad_sin = std::sin(solar_radius);
    state->ar2                = hk_real(1) / (sol_rad_sin * sol_rad_sin);

    //   Finally, we have to reduce the scaling factor of the sky by the
    //   ratio used to scale the solar disc size. The rationale behind this is
    //   that the scaling factors apply to the new blackbody spectrum, which
    //   can be more or less bright than the one our sun emits. However, we
    //   just scaled the size of the alien solar disc so it is roughly as
    //   bright (in terms of energy emitted) as the terrestrial sun. So the sky
    //   dome has to be reduced in brightness appropriately - but not in an
    //   uniform fashion across wavebands. If we did that, the sky colour would
    //   be wrong.

    for (unsigned int i = 0; i < 11; i++) {
        state->emission_correction_factor_sky[i] = solar_intensity *
                                                   state->emission_correction_factor_sun[i] / ratio;
    }

    return state;
}

void arhosekskymodelstate_free(ArHosekSkyModelState* state) {
    free(state);
}

hk_real arhosekskymodel_radiance(ArHosekSkyModelState* state,
                                 //	hk_real                  theta,
                                 hk_real cos_theta, hk_real sqrt_cos_theta, hk_real gamma,
                                 hk_real cos_gamma, hk_real wavelength) {
    int low_wl = static_cast<int>((wavelength - hk_real(320)) / hk_real(40));

    if (low_wl < 0 || low_wl >= 11) {
        return 0.0f;
    }

    hk_real interp = std::fmod((wavelength - hk_real(320)) / hk_real(40), hk_real(1));

    hk_real val_low = ArHosekSkyModel_GetRadianceInternal(state->configs[low_wl],
                                                          //       theta,
                                                          cos_theta, sqrt_cos_theta, gamma,
                                                          cos_gamma) *
                      state->radiances[low_wl] * state->emission_correction_factor_sky[low_wl];

    if (interp < 1e-6) return val_low;

    hk_real result = (hk_real(1) - interp) * val_low;

    if (low_wl + 1 < 11) {
        result += interp *
                  ArHosekSkyModel_GetRadianceInternal(state->configs[low_wl + 1],
                                                      //        theta,
                                                      cos_theta, sqrt_cos_theta, gamma, cos_gamma) *
                  state->radiances[low_wl + 1] * state->emission_correction_factor_sky[low_wl + 1];
    }

    return result;
}

// xyz and rgb versions

ArHosekSkyModelState* arhosek_xyz_skymodelstate_alloc_init(const hk_real turbidity,
                                                           const hk_real albedo,
                                                           const hk_real elevation) {
    ArHosekSkyModelState* state = ALLOC(ArHosekSkyModelState);

    //	state->solar_radius = hk_real(TERRESTRIAL_SOLAR_RADIUS);
    const hk_real solar_radius = hk_real(TERRESTRIAL_SOLAR_RADIUS);
    const hk_real sol_rad_sin  = std::sin(solar_radius);
    state->ar2                 = hk_real(1) / (sol_rad_sin * sol_rad_sin);
    state->turbidity           = turbidity;
    state->albedo              = albedo;
    state->elevation           = elevation;

    for (unsigned int channel = 0; channel < 3; ++channel) {
        ArHosekSkyModel_CookConfiguration(datasetsXYZ[channel], state->configs[channel], turbidity,
                                          albedo, elevation);

        state->radiances[channel] = ArHosekSkyModel_CookRadianceConfiguration(
            datasetsXYZRad[channel], turbidity, albedo, elevation);
    }

    return state;
}

ArHosekSkyModelState* arhosek_rgb_skymodelstate_alloc_init(const hk_real turbidity,
                                                           const hk_real albedo,
                                                           const hk_real elevation) {
    ArHosekSkyModelState* state = ALLOC(ArHosekSkyModelState);

    //	state->solar_radius = hk_real(TERRESTRIAL_SOLAR_RADIUS);
    const hk_real solar_radius = hk_real(TERRESTRIAL_SOLAR_RADIUS);
    const hk_real sol_rad_sin  = std::sin(solar_radius);
    state->ar2                 = hk_real(1) / (sol_rad_sin * sol_rad_sin);
    state->turbidity           = turbidity;
    state->albedo              = albedo;
    state->elevation           = elevation;

    for (unsigned int channel = 0; channel < 3; ++channel) {
        ArHosekSkyModel_CookConfiguration(datasetsRGB[channel], state->configs[channel], turbidity,
                                          albedo, elevation);

        state->radiances[channel] = ArHosekSkyModel_CookRadianceConfiguration(
            datasetsRGBRad[channel], turbidity, albedo, elevation);
    }

    return state;
}

hk_real arhosek_tristim_skymodel_radiance(ArHosekSkyModelState* state,
                                          //	hk_real                  theta,
                                          hk_real cos_theta, hk_real sqrt_cos_theta, hk_real gamma,
                                          hk_real cos_gamma, int channel) {
    return ArHosekSkyModel_GetRadianceInternal(state->configs[channel],
                                               //	theta,
                                               cos_theta, sqrt_cos_theta, gamma, cos_gamma) *
           state->radiances[channel];
}

constexpr int pieces = 45;
constexpr int order  = 4;

hk_real pow3(hk_real x) {
    return x * x * x;
}

void arhosekskymodel_solar_radiance_temp(ArHosekSkyModelSolarTemp* temp, hk_real theta,
                                         hk_real sin_gamma) {
    hk_real elevation = ((hk_real(MATH_PI) / hk_real(2)) - theta);

    int pos = (int)(std::cbrt(hk_real(2) * elevation / hk_real(MATH_PI)) * pieces);  // floor

    if (pos > 44) pos = 44;

    const hk_real break_x = pow3(((hk_real)pos / (hk_real)pieces)) *
                            (hk_real(MATH_PI) * hk_real(0.5));

    const hk_real sin_gamma_squared = sin_gamma * sin_gamma;

    temp->elevation_minus_break_x = elevation - break_x;
    temp->sin_gamma_squared       = sin_gamma_squared;
    temp->pos_plus_1              = pos + 1;
}

hk_real arhosekskymodel_sr_internal(ArHosekSkyModelState* state, ArHosekSkyModelSolarTemp* temp,
                                    int turbidity, int wl) {
    const hk_real* coefs = solarDatasets[wl] +
                           ((order * pieces) * turbidity + order * (temp->pos_plus_1) - 1);

    hk_real       res   = 0.0;
    const hk_real x     = temp->elevation_minus_break_x;
    hk_real       x_exp = 1.0;

    for (int i = 0; i < order; ++i) {
        res += x_exp * *coefs--;
        x_exp *= x;
    }

    return res * state->emission_correction_factor_sun[wl];
}

hk_real arhosekskymodel_solar_radiance_internal2(ArHosekSkyModelState*     state,
                                                 ArHosekSkyModelSolarTemp* temp,
                                                 hk_real                   wavelength) {
    assert(wavelength >= 320.0 && wavelength <= 720.0 && state->turbidity >= 1.0 &&
           state->turbidity <= 10.0);

    int     turb_low  = (int)state->turbidity - 1;
    hk_real turb_frac = state->turbidity - (hk_real)(turb_low + 1);

    if (turb_low == 9) {
        turb_low  = 8;
        turb_frac = 1.0;
    }

    int     wl_low  = (int)((wavelength - 320.0) / 40.0);
    hk_real wl_frac = std::fmod(wavelength, hk_real(40)) / hk_real(40);

    if (wl_low == 10) {
        wl_low  = 9;
        wl_frac = 1.0;
    }

    hk_real direct_radiance =
        (hk_real(1) - turb_frac) *
            ((hk_real(1) - wl_frac) * arhosekskymodel_sr_internal(state, temp, turb_low, wl_low) +
             wl_frac * arhosekskymodel_sr_internal(state, temp, turb_low, wl_low + 1)) +
        turb_frac * ((hk_real(1) - wl_frac) *
                         arhosekskymodel_sr_internal(state, temp, turb_low + 1, wl_low) +
                     wl_frac * arhosekskymodel_sr_internal(state, temp, turb_low + 1, wl_low + 1));

    hk_real ldCoefficient[6];

    for (int i = 0; i < 6; i++)
        ldCoefficient[i] = (hk_real(1) - wl_frac) * limbDarkeningDatasets[wl_low][i] +
                           wl_frac * limbDarkeningDatasets[wl_low + 1][i];

    // sun distance to diameter ratio, squared

    //	const hk_real sol_rad_sin = std::sin(state->solar_radius);
    //	const hk_real ar2 = hk_real(1) / ( sol_rad_sin * sol_rad_sin );
    //	const hk_real sin_gamma = std::sin(gamma);
    hk_real sc2 = hk_real(1) - state->ar2 * temp->sin_gamma_squared;
    if (sc2 < 0.0) sc2 = 0.0;
    hk_real sampleCosine = std::sqrt(sc2);

    //   The following will be improved in future versions of the model:
    //   here, we directly use fitted 5th order polynomials provided by the
    //   astronomical community for the limb darkening effect. Astronomers need
    //   such accurate fittings for their predictions. However, this sort of
    //   accuracy is not really needed for CG purposes, so an approximated
    //   dataset based on quadratic polynomials will be provided in a future
    //   release.

    hk_real sampleCosine2 = sampleCosine * sampleCosine;
    hk_real sampleCosine3 = sampleCosine2 * sampleCosine;
    hk_real sampleCosine4 = sampleCosine3 * sampleCosine;
    hk_real sampleCosine5 = sampleCosine4 * sampleCosine;

    hk_real darkeningFactor = ldCoefficient[0] + ldCoefficient[1] * sampleCosine +
                              ldCoefficient[2] * sampleCosine2 + ldCoefficient[3] * sampleCosine3 +
                              ldCoefficient[4] * sampleCosine4 + ldCoefficient[5] * sampleCosine5;

    direct_radiance *= darkeningFactor;

    return direct_radiance;
}

hk_real arhosekskymodel_solar_radiance(ArHosekSkyModelState* state, ArHosekSkyModelSolarTemp* temp,
                                       hk_real cos_theta, hk_real sqrt_cos_theta, hk_real gamma,
                                       hk_real cos_gamma, hk_real wavelength) {
    hk_real direct_radiance = arhosekskymodel_solar_radiance_internal2(state, temp, wavelength);

    hk_real inscattered_radiance = arhosekskymodel_radiance(state, cos_theta, sqrt_cos_theta, gamma,
                                                            cos_gamma, wavelength);

    return direct_radiance + inscattered_radiance;
}
