
cmake_minimum_required(VERSION 3.15)

project(LIB3)

#源码路径
aux_source_directory(./ SRC)
#包含文件
include_directories(${INCLUDE_PATH})
#输出目录
set(LIBRARY_OUTPUT_PATH ${LIB_OUTPUT_PATH})

#使用第三方库
link_opencv()

#生成库
add_library(${LIB3_NAME} STATIC ${SRC})
