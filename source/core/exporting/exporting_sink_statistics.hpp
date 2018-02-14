#ifndef SU_CORE_EXPORTING_SINK_STATISTICS_HPP
#define SU_CORE_EXPORTING_SINK_STATISTICS_HPP

#include "exporting_sink.hpp"

namespace image { class Writer; }

namespace exporting {

class Statistics : public Sink {

public:

	Statistics();
	~Statistics();

	virtual void write(const image::Float4& image, uint32_t frame,
					   thread::Pool& pool) override final;
};

}

#endif
