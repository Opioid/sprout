#pragma once

#include "vector3.hpp"
#include "vector4.hpp"

namespace math {

using Plane = Vector4f_a;

namespace plane {

Plane create(FVector3f_a normal, float d);
Plane create(FVector3f_a normal, FVector3f_a point);
Plane create(FVector3f_a v0, FVector3f_a v1, FVector3f_a v2);

float dot(FVector4f_a p, FVector3f_a v);

bool behind(FVector4f_a p, FVector3f_a point);

}}
