
cmake_minimum_required(VERSION 3.15)

set(CUDA_ARCHITECTURES 70 75 80 86)

enable_language(CUDA)

set(CUDA_STANDARD 12)

#设置变量
#引用外部库,必须放在add_subdirectory之前
#CUDA 库路径
set(CUDA_INLCUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin/include)
set(CUDA_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)
#OpenCV 库名
set(CUDA_LIB_NAME opencv_world480d)

#链接opencv
function(link_cuda)
    link_directories(${CUDA_LIB_PATH})
    link_libraries(${CUDA_LIB_NAME})
endfunction()

function(include_cuda)
#项目自身包含外部库目录
    include_directories(${CUDA_INLCUDE_PATH} ${CUDA_INLCUDE_PATH})
endfunction()
