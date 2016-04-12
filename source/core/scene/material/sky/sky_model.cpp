#include "sky_model.hpp"

namespace scene { namespace material { namespace sky {

void Model::set_ground_albedo(math::pfloat3 albedo) {
	ground_albedo_ = albedo;
}

void Model::set_turbidity(float turbidity) {
	turbidity_ = turbidity;
}

}}}
