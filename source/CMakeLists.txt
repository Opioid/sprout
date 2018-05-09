cmake_minimum_required(VERSION 3.1)
project(sprout)

if (CMAKE_VERSION VERSION_LESS 3.3)
	# This only exists to keep travis cmake version from complaining
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else ()
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif ()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# On Linux this should give -fPIC/fPIE
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(NAN "Soft assert on NaN" OFF)
option(Valgrind "Valgrind compatible build" OFF)

if (${NAN})
	add_definitions(-DSU_DEBUG)
endif ()

if (UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif ()

if (LINUX) 
	# linux stuff here
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -W -Wall -Wextra -fno-rtti")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsave-optimization-record")

	if (${Valgrind})
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3")
	else ()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
	endif ()
	
	#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

	#strip debug symbols
	#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
	if (CMAKE_BUILD_TYPE MATCHES RELEASE)
		set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsave-optimization-record")
	endif (CMAKE_BUILD_TYPE MATCHES RELEASE)

	# Enable specific extra warnings
	# add_compile_options(-Wuseless-cast)
	# add_compile_options(-Wshadow)
elseif (MSVC)
	# set(CMAKE_CXX_STANDARD 17) doesn't seem to work for visual studio target at the moment
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")

	if (CMAKE_CXX_FLAGS MATCHES "/GR")
		string(REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()

	# Force to always compile with W4
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else ()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif ()

	# disable specific warnings
	add_compile_options(/wd4201) # nameless struct/union extension
	add_compile_options(/wd4324) # pad due to alignment specifier
	add_compile_options(/wd4505) # unreferenced local function has been removed
	add_compile_options(/wd4514) # unreferenced inline function has been removed
    add_compile_options(/wd4582) # constructor is not implicitly called
    add_compile_options(/wd4587) # constructor is no longer implicitly called
    add_compile_options(/wd4625) # copy operator implicitly deleted
    add_compile_options(/wd4626) # assignment operator implicitly deleted
    add_compile_options(/wd4710) # function not inlined
    add_compile_options(/wd4711) # function selected for automatic inline expansion
	# add_compile_options(/wd4820) # bytes' bytes padding added after construct
    add_compile_options(/wd5026) # move constructor operator implicitly deleted
    add_compile_options(/wd5027) # move assignment operator implicitly deleted

	# To avoid warning spam originates from rapidjson
	add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
endif ()

include(base/base.cmake)
include(core/core.cmake)
include(extension/extension.cmake)
include(cli/cli.cmake)  
