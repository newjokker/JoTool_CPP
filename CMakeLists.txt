cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(example-app)


# 不知道为什么要这一行，没有的话报错，to string 不是 std 的函数之类的
SET(CMAKE_CXX_FLAGS "-std=c++11 -O3")


# 指定头文件搜索路径
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}
     "/usr/include"
	"/usr/del/example-app/tools/"
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")


#find_package( CUDA REQUIRED )

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)


# 设置静态库文件目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
# 动态库文件目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
# 可执行文件目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)


#set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} "~/libtorch/libtorch")
set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} "/home/libtorch" )
set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} "/usr/local/cuda-10.2" )

# find_package(Torch REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# SET(Torch_DIR /usr/del/torch/share/cmake/Torch)

find_package( OpenCV REQUIRED )

FIND_PACKAGE(nlohmann_json REQUIRED)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRCLIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/tools SRCLIST)

# add_library(saturntools SHARED ${SRCLIST})   
add_library(saturntools STATIC ${SRCLIST})   

add_executable(test main.cpp)


target_link_libraries(
    test 
    # ${TORCH_LIBRARIES}
    ${OpenCV_LIBS}
    # curlpp
    # curl
    saturntools
	nlohmann_json::nlohmann_json
)
    

set_property(TARGET test PROPERTY CXX_STANDARD 14)



MESSAGE(STATUS, "------------------------------------------------------------")

MESSAGE("${TORCH_LIBRARIES}")

MESSAGE("${OpenCV_LIBS}")

MESSAGE("${PROJECT_SOURCE_DIR}")

MESSAGE("${TORCH_INCLUDE_DIRS}")

MESSAGE("${OpenCV_INCLUDE_DIRS}")

MESSAGE(STATUS, "------------------------------------------------------------")


# ---------------------- 安装规则 --------------------------------

# refer : https://www.hahack.com/codes/cmake/
# install (TARGETS MathFunctions DESTINATION bin)

# install (FILES MathFunctions.h DESTINATION include)


# ---------------------- 测试规则 --------------------------------




