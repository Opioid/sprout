# https://www.mattkeeter.com/blog/2018-01-06-versioning/
include(git_version.cmake)

# Add a custom command that produces version.cpp, plus
# a dummy output that's not actually produced, in order
# to force version.cmake to always be re-run before the build
ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.hpp
           ${CMAKE_CURRENT_BINARY_DIR}/_version.hpp
    COMMAND ${CMAKE_COMMAND} -P
            ${CMAKE_CURRENT_SOURCE_DIR}/git_version.cmake)

target_sources(base
  PRIVATE
  "platform.cpp"
  "platform.hpp"
)  
