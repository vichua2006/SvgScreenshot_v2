cmake_minimum_required(VERSION 3.16)
project(ScreenShot VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Check if we're on Windows and set appropriate generator
if(WIN32)
    # Try to find MinGW first, then fall back to other options
    if(NOT CMAKE_CXX_COMPILER)
        find_program(MINGW_GXX NAMES x86_64-w64-mingw32-g++ mingw32-g++ g++)
        if(MINGW_GXX)
            set(CMAKE_CXX_COMPILER ${MINGW_GXX})
            message(STATUS "Found MinGW compiler: ${MINGW_GXX}")
        endif()
    endif()
endif()

# Find OpenCV package
find_package(OpenCV REQUIRED)
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. Please install OpenCV first.")
endif()

# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

# Set source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_executable(screenshot ${SOURCES})

# Link OpenCV libraries
target_link_libraries(screenshot ${OpenCV_LIBS})

# Link Windows GDI libraries (equivalent to the Makefile LDFLAGS)
if(WIN32)
    target_link_libraries(screenshot 
        gdi32 
        ole32 
        uuid 
        comdlg32 
        shell32 
        msimg32
    )
endif()

# Set compiler flags (equivalent to CXXFLAGS in Makefile)
target_compile_options(screenshot PRIVATE 
    $<$<CXX_COMPILER_ID:GNU>:-Wall -O2>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -O2>
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /O2>
)

# Print configuration info
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
message(STATUS "Source files: ${SOURCES}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}") 