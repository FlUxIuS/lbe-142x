cmake_minimum_required(VERSION 3.10)
project(lbe-142x C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add source files
if(WIN32)
    set(SOURCES
        src/main.c
        src/lbe_device_windows.c
    )
else()
    set(SOURCES
        src/main.c
        src/lbe_device_linux.c
    )
endif()

# Add header files
set(HEADERS
    include/lbe_common.h
    include/lbe_device.h
)

set(CMAKE_EXE_LINKER_FLAGS "-s")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Platform-specific libraries and flags
if(WIN32)
    # Windows-specific settings Extract the contents of libusb-1.0.27.7z(or more) to a `libusb` directory in the root of the project
	set(LIBUSB_ROOT "${PROJECT_SOURCE_DIR}/libusb" CACHE PATH "Path to LibUSB root directory")
	set(LIBUSB_INCLUDE_DIR "${LIBUSB_ROOT}/include")
    if(MINGW)
        # MinGW64 specific settings
		set(LIBUSB_LIBRARY_DIR "${LIBUSB_ROOT}/MinGW64/static")
    elseif(MSVC)
        # Visual Studio 2022 specific settings
        set(LIBUSB_LIBRARY_DIR "${LIBUSB_ROOT}/VS2022/MS64/static")
    endif()
	message(STATUS "LIBUSB_INCLUDE_DIR: ${LIBUSB_INCLUDE_DIR}")
	find_library(LIBUSB_LIBRARY NAMES "libusb-1.0" PATHS ${LIBUSB_LIBRARY_DIR} NO_DEFAULT_PATH)
	message(STATUS "LIBUSB_LIBRARY: ${LIBUSB_LIBRARY}")

	if(NOT EXISTS ${LIBUSB_INCLUDE_DIR}/libusb.h)
		message(FATAL_ERROR "LibUSB include directory not found. Please extract LibUSB to ${LIBUSB_ROOT}")
	endif()
	
	if(NOT EXISTS ${LIBUSB_LIBRARY})
		message(FATAL_ERROR "LibUSB library not found. Please extract LibUSB to ${LIBUSB_ROOT}")
	endif()
	
	include_directories(${LIBUSB_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${LIBUSB_LIBRARY})
elseif(UNIX AND NOT APPLE)
    # Linux-specific settings
    target_link_libraries(${PROJECT_NAME} udev)
endif()

# Compiler-specific options
if(MSVC)
    # Visual Studio specific flags
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
    # Use static runtime for both release and debug configurations
    foreach(flag_var
            CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif()
        if(${flag_var} MATCHES "/MDd")
            string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()
else()
    # GCC/Clang flags (for MinGW and Linux)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-pedantic -Werror)
    #target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Print build information
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER_VERSION: ${CMAKE_CXX_COMPILER_VERSION}")