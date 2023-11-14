
cmake_minimum_required(VERSION 3.15)

project(TEST2)

#源码路径
aux_source_directory(./ SRC)
#包含文件
include_directories(${INCLUDE_PATH})
#输出目录

set(EXECUTABLE_OUTPUT_PATH ${BIN_OUTPUT_PATH})

#引用外部库
#link_directories(${OPENCV_LIB_PATH})
#link_libraries(${OPENCV_LIB_NAME})

message(STATUS "OPENCV_LIB: ${OPENCV_LIB_PATH} ${OPENCV_LIB_NAME}.lib")

#include_directories(${OPENCV_INCLUDE_DIR} ${OPENCV2_INLCUDE_DIR})


#set(CMAKE_BUILD_TYPE Release)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    message("BUILD TYPE Debug")
    link_directories(${LIB_OUTPUT_PATH}/Debug)
elseif(CMAKE_BUILD_TYPE MATCHES "Release")
    link_directories(${LIB_OUTPUT_PATH}/Release)
endif()

#链接内部库
foreach(LIB_NAME ${LINK_INTERNAL_LIB_NAMES})
    message(${LIB_NAME})
    link_libraries(${LIB_NAME})
endforeach()

#再链接一次opencv
link_opencv()

#生成可执行文件
add_executable(${TEST2EXC_NAME} ${SRC})