cmake_minimum_required (VERSION 3.8)

# ffmpeg目录
set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/ffmpeg)
# 包含目录
include_directories(${FFMPEG_DIR}/include)
# 库目录
link_directories(${FFMPEG_DIR}/lib)
# 设置exe输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

if(MSVC)
	add_compile_options(/utf8)
endif()

add_subdirectory ("ffmpeg_demo")
