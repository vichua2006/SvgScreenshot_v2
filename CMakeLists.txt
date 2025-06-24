cmake_minimum_required(VERSION 3.14)
project(SvgScreenshot_v2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    opencv
    GIT_REPOSITORY https://github.com/opencv/opencv.git
    GIT_TAG 4.8.0
)
FetchContent_MakeAvailable(opencv)

add_executable(screenshot
    src/main.cpp
    src/displayWindow.cpp
)

target_include_directories(screenshot PRIVATE src)

target_link_libraries(screenshot PRIVATE
    gdi32 ole32 uuid comdlg32 shell32 msimg32
    opencv_core opencv_imgcodecs opencv_imgproc
)

find_package(OpenCV QUIET)
if(OpenCV_FOUND)
    target_include_directories(screenshot PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(screenshot PRIVATE ${OpenCV_LIBS})
    target_compile_definitions(screenshot PRIVATE USE_OPENCV)
endif()
