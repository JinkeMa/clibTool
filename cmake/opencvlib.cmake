
cmake_minimum_required(VERSION 3.15)

#  opencv_highgui450 opencv_core450 opencv_imgproc450 opencv_imgcodecs450

#设置变量
#引用外部库,必须放在add_subdirectory之前
#OpenCV 库路径
set(OPENCV_INLCUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin/include)
set(OPENCV2_INLCUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin/include/opencv2)
set(OPENCV_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)
#OpenCV 库名
set(OPENCV_LIB_NAME opencv_world480d)

#链接opencv
function(link_opencv)
    link_directories(${OPENCV_LIB_PATH})
    link_libraries(${OPENCV_LIB_NAME})
endfunction()

function(include_opencv)
#项目自身包含外部库目录
    include_directories(${OPENCV_INLCUDE_PATH} ${OPENCV2_INLCUDE_PATH})
endfunction()

include_opencv()