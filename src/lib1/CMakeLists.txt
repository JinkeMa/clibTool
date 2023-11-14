
cmake_minimum_required(VERSION 3.15)

project(LIB1)

#源码路径
aux_source_directory(./ SRC)
#包含文件
include_directories(${INCLUDE_PATH})
#输出目录
set(LIBRARY_OUTPUT_PATH ${LIB_OUTPUT_PATH})

#lib1依赖的库
message("LIB1 linking OPENCV_LIB_PATH " ${OPENCV_LIB_PATH})
link_opencv()

#生成库
add_library(${LIB1_NAME} STATIC ${SRC})