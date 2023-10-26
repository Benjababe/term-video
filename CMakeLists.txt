cmake_minimum_required(VERSION 3.6)
project(cpp_video_to_ascii VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out/build/)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)
include_directories(${PROJECT_SOURCE_DIR}/ffmpeg/include)
link_directories(${PROJECT_SOURCE_DIR}/ffmpeg/lib)

file(GLOB ALL_SRCS
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${PROJECT_NAME} ${ALL_SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})

if(CMAKE_HOST_SYSTEM MATCHES Linux)
    find_package(Curses REQUIRED)
    include_directories(${CURSES_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CURSES_LIBRARIES})
endif()