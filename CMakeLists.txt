cmake_minimum_required(VERSION 3.6)

# Edit in include/media.hpp as well
# set(MEDIA_HANDLER opencv)
set(MEDIA_HANDLER ffmpeg)

set(PROJECT_NAME term_video_${MEDIA_HANDLER})
project(${PROJECT_NAME} CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out/build/)

find_package(PkgConfig REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

file(GLOB ALL_SRCS
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${PROJECT_NAME} ${ALL_SRCS})

pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    ao
    libavformat
    libavcodec
    libavutil
    libswresample
    libswscale)

target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::LIBAV)

if(MEDIA_HANDLER MATCHES opencv)
    pkg_check_modules(OPENCV IMPORTED_TARGET
        opencv4)

    target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::OPENCV)
endif()

if(CMAKE_HOST_SYSTEM MATCHES Linux)
    find_package(Curses REQUIRED)
    include_directories(${CURSES_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CURSES_LIBRARIES})
endif()