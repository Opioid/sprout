target_sources(core
  PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/progress_sink_null.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/progress_sink_null.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/progress_sink_std_out.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/progress_sink_std_out.hpp"
  "${CMAKE_CURRENT_LIST_DIR}/progress_sink.hpp"
)
