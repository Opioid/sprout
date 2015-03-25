#pragma once

namespace rendering { namespace material {

class Sample;

class Material {
public:

	virtual ~Material() {}

	virtual const Sample& sample() = 0;
};

}}
