cmake_minimum_required(VERSION 3.13)
project(sprout)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# On Linux this should give -fPIC/fPIE
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)

option(ACEScg "Render in ACEScg color space instead of sRGB" ON)
option(IBL_MIS_Compensation "Enable MIS compensation technique for IBLs" ON)
option(NAN "Soft assert on NaN" OFF)
option(ZSTD "Build with ZSTD support" ON)
option(Valgrind "Valgrind compatible build" OFF)
option(IWYU "Inlcude-what-you-use" OFF)
option(ASAN "clang adress-sanitizer" OFF)
option(MSAN "clang memory-sanitizer" OFF)

if (${ACEScg})
	add_definitions(-DSU_ACESCG)
endif ()

if (${IBL_MIS_Compensation})
	add_definitions(-DSU_IBL_MIS_COMPENSATION)
endif ()

if (${NAN})
	add_definitions(-DSU_DEBUG)
endif ()

if (${ZSTD})
    add_definitions(-DSU_ZSTD)
endif()

if (${IWYU})
    find_program(IWYU_PATH NAMES include-what-you-use iwyu)
    if(NOT IWYU_PATH)
        message(FATAL_ERROR "Could not find the program include-what-you-use")
    endif()
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
    set(CMAKE_C_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
endif()

if (${ASAN})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
endif()

if (${MSAN})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=memory -fsanitize-memory-track-origins")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=memory -fsanitize-memory-track-origins")
endif()

if (UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif ()

if (LINUX) 
	# linux stuff here
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -W -Wall -Wextra -fno-rtti -fno-exceptions")

#    if (NOT ${NAN})
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
#    endif ()

	if (${Valgrind})
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -mf16c")
	else ()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
	endif ()
	
	#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

	#strip debug symbols -- doesn't work?
	if (CMAKE_BUILD_TYPE MATCHES RELEASE)
		set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -s")  ## Strip binary
		# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsave-optimization-record")
	endif (CMAKE_BUILD_TYPE MATCHES RELEASE)

	# Enable specific extra warnings
	# add_compile_options(-Wuseless-cast)
	# add_compile_options(-Wshadow)
elseif (MSVC)
#    if (NOT ${NAN})
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast")
#    endif ()

	if (CMAKE_CXX_FLAGS MATCHES "/GR")
		string(REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()

    # Disable exceptions
    if (CMAKE_CXX_FLAGS MATCHES "/EHsc")
        string(REGEX REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()
    add_definitions(-D_HAS_EXCEPTIONS=0)

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
    add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)
endif ()

add_subdirectory(base)
add_subdirectory(core)
add_subdirectory(extension)
add_subdirectory(capi)
add_subdirectory(capi-test)
add_subdirectory(cli)
add_subdirectory(it)
