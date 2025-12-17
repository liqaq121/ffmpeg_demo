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

set(Include_Dirs 
.
main
review
)

file(GLOB dirs LIST_DIRECTORIES true "*")
foreach(dir ${dirs})
    if(IS_DIRECTORY ${dir})
        list(APPEND Include_Dirs ${dir})
    endif()
endforeach()

target_include_directories(${PROJECT_NAME} PRIVATE ${Include_Dirs})