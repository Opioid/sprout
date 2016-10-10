#pragma once

#include "base/math/vector.hpp"
#include "base/spectrum/discrete.hpp"

struct ArHosekSkyModelState;

namespace procedural { namespace sky {

class Model {

public:

	Model();
	~Model();

	// Return signals whether there was a change (not success/failure!)
	bool init();

	float3 sun_direction() const;
	void set_sun_direction(float3_p direction);

	void set_ground_albedo(float3_p albedo);
	void set_turbidity(float turbidity);

	float3 evaluate_sky(float3_p wi) const;

	float3 evaluate_sky_and_sun(float3_p wi) const;

	static float3 zenith();

private:

	void release();

	float3 sun_direction_;
	float3 ground_albedo_;

	float turbidity_;

	static constexpr uint32_t Num_bands = 6;

	ArHosekSkyModelState* skymodel_states_[Num_bands];

	bool dirty_;

	static const float3 zenith_;

	using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;
};

}}
