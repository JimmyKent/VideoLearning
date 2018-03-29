
cmake_minimum_required(VERSION 3.9)
project(VideoLearning)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# /usr/local/Cellar/ffmpeg/3.4.1
include_directories(/usr/local/Cellar/ffmpeg/3.4.1/include/)
link_directories(/usr/local/Cellar/ffmpeg/3.4.1/lib/)

# /usr/local/Cellar/sdl2/2.0.7
include_directories(/usr/local/Cellar/sdl2/2.0.7/include/)
link_directories(/usr/local/Cellar/sdl2/2.0.7/lib/)


# error,change find_package to find_library
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
#find_package(SDL2 REQUIRED)
#include_directories(${SDL2_INCLUDE_DIR})
#link_directories(${SDL2_LIBRARY})

find_library(SDL2_LIBRARY SDL2 "/usr/local/Cellar/sdl2/2.0.7/include")
#find_library(SDL2_App ${SDL2_LIBRARY})


add_executable(VideoLearning ${SOURCE_FILES}
        main.cpp
        Video2YUV.cpp StringUtils.cpp StringUtils.h)

target_link_libraries(
        VideoLearning
        avcodec
        avdevice
        avfilter
        avformat
        avresample
        avutil
        postproc
        swresample
        swscale
        ${SDL2_LIBRARY}
)

