cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(example-app)


# 不知道为什么要这一行，没有的话报错，to string 不是 std 的函数之类的
SET(CMAKE_CXX_FLAGS "-std=c++11 -O3")


# 指定头文件搜索路径
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}
    "/usr/include"
    "/usr/include/mysql"
	"/usr/lib/x86_64-linux-gnu"
)


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

SET(OpenCV_STATIC_LIBS
	/usr/local/lib/libopencv_core.a
	/usr/local/lib/libopencv_imgcodecs.a
	/usr/local/lib/libopencv_imgproc.a
	/usr/lib/x86_64-linux-gnu/libpng.a
	/usr/lib/x86_64-linux-gnu/libjpeg.a
	)

SET(other_STATIC_LIBS
	/usr/lib/gcc/x86_64-linux-gnu/5/libstdc++.a
	/usr/lib/x86_64-linux-gnu/libjasper.a
	)


# 动态库文件目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
# 可执行文件目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)


set(MYSQL_INCLUDE_DIR "/usr/include/mysql")
set(MYSQL_LIB_DIR "/usr/lib/x86_64-linux-gnu")
include_directories(${MYSQL_INCLUDE_DIR})


find_package(OpenCV REQUIRED )
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)

find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRCLIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/tools SRCLIST)


set(OPENSSL_INCLUDE_DIR "/usr/include/openssl")
set(OPENSSL_LIB_DIR "/usr/lib/x86_64-linux-gnu")
include_directories(${OPENSSL_INCLUDE_DIR})


# add_library(saturntools_v3.0.6 STATIC ${SRCLIST})   
add_library(saturn_ucd_tools STATIC ${SRCLIST})   

add_executable(ucd main.cpp)
link_directories(${OpenCV_LIB_DIR})

target_link_libraries(
    ucd 
	
	${OpenCV_LIBS}
    saturn_ucd_tools
	nlohmann_json::nlohmann_json
	
	${OpenCV_STATIC_LIBS}
	${CMAKE_DL_LIBS}
	${JPEG_LIBRARIES}
	PNG::PNG
		

	${MYSQL_LIB_DIR}/libmysqlclient.a
	
	${OPENSSL_LIB_DIR}/libssl.a 
	${OPENSSL_LIB_DIR}/libcrypto.a
	
	${other_STATIC_LIBS}
	
  
    # Link against libgcc_s.a
    -static-libgcc
  
	
)


set_property(TARGET ucd PROPERTY CXX_STANDARD 14)