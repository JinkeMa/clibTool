# CMakeList.txt: clibTool 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.15)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("clibTool")

#设置全局变量给子目录用

#设置包含路径
set(INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

#设置库名字
set(LIB1_NAME lib1)
set(LIB2_NAME lib2)
set(LIB3_NAME lib3)

#设置综合链接库名字
list(APPEND LINK_INTERNAL_LIB_NAMES ${LIB1_NAME} ${LIB2_NAME} ${LIB3_NAME})

#测试程序名字
set(TEST1EXC_NAME test1)
set(TEST2EXC_NAME test2)

#设置库生成目录
set(LIB_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/lib)

#设置测试程序生成目录
set(BIN_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)


#设置.cmake文件的目录
set(CMAKE_FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
#
include(${CMAKE_FILE_PATH}/opencvlib.cmake)
include(${CMAKE_FILE_PATH}/cudalib.cmake)


#添加子项目,必须与文件夹名字相同,每个CMakeLists.txt必须直接位于子目录下
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/${LIB1_NAME})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/${LIB2_NAME})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/${LIB3_NAME})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/${TEST1EXC_NAME})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/${TEST2EXC_NAME})


message("opencv dir:" ${OPENCV_INLCUDE_PATH})

message("cuda:" ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})