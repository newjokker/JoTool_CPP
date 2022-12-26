cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(example-app)


# 不知道为什么要这一行，没有的话报错，to string 不是 std 的函数之类的
SET(CMAKE_CXX_FLAGS "-std=c++11 -O3")


# 指定头文件搜索路径
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}
    "/usr/include"
	# "/home/ldq/Apps_jokker/so_dir"
    "/usr/include/mysql"
)

# link_directories("/usr/lib/x86_64-linux-gnu")


set(MYSQL_LIBS mysqlclient pthread z m rt atomic ssl crypto dl)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")


#find_package( CUDA REQUIRED )

#SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
#SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 设置静态库文件目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "/home/ldq/Apps_jokker/so_dir")
# 动态库文件目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
# 指定固定的动态库路径，约定俗成的路径
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "/home/ldq/Apps_jokker/so_dir")
# 可执行文件目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/home/ldq/Apps_jokker")


#set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} "~/libtorch/libtorch")
set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} "/home/libtorch" )
set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} "/usr/local/cuda-10.2" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# find_package(Torch REQUIRED)
find_package( OpenCV REQUIRED )
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRCLIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/tools SRCLIST)

add_library(saturntools_v2.4.5 SHARED ${SRCLIST})   
# add_library(saturntools STATIC ${SRCLIST})   

add_executable(ucd main.cpp)

target_link_libraries(
    ucd 
    # ${TORCH_LIBRARIES}
    ${OpenCV_LIBS}
    OpenSSL::Crypto OpenSSL::SSL
    saturntools_v2.4.5
	nlohmann_json::nlohmann_json
    ${MYSQL_LIBS}
)
    

set_property(TARGET ucd PROPERTY CXX_STANDARD 14)



# MESSAGE(STATUS, "------------------------------------------------------------")

# MESSAGE("${TORCH_LIBRARIES}")

# MESSAGE("${OpenCV_LIBS}")

# MESSAGE("${PROJECT_SOURCE_DIR}")

# MESSAGE("${TORCH_INCLUDE_DIRS}")

# MESSAGE("${OpenCV_INCLUDE_DIRS}")

# MESSAGE(STATUS, "------------------------------------------------------------")


# ---------------------- 安装规则 --------------------------------

# refer : https://www.hahack.com/codes/cmake/
# install (TARGETS MathFunctions DESTINATION bin)

# install (FILES MathFunctions.h DESTINATION include)


# ---------------------- 测试规则 --------------------------------




