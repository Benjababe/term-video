cmake_minimum_required(VERSION 3.0.0)
project(cpp_video_to_ascii VERSION 1.0.0 LANGUAGES CXX)

find_package(OpenCV REQUIRED)
find_package(Curses REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CURSES_INCLUDE_DIR})
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

file(GLOB ALL_SRCS
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.c"
)

add_executable(${PROJECT_NAME} ${ALL_SRCS})

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})

set(CMAKE_CXX_STANDARD 11)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out/build/)