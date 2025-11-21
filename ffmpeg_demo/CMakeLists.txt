cmake_minimum_required (VERSION 3.8)

project(ffmpeg_demo)

file(GLOB_RECURSE SRC 
${CMAKE_CURRENT_SOURCE_DIR}/*.h
${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable (${PROJECT_NAME}
${SRC}
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
avformat
avcodec
avutil
swresample
swscale
avfilter
)
