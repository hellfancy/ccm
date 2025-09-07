cmake_minimum_required(VERSION 3.20)
project(ccm)

set(CMAKE_CXX_STANDARD 17)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug") 
	set(CMAKE_CXX_FLAGS_RELEASE"-O0 -g")
else()
	set(CMAKE_C_FLAGS_RELEASE "-O2 -s -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -s -DNDEBUG")
	#add_definitions(-O2)
endif()

# 将所有 .cpp 文件列出来
file(GLOB_RECURSE CXX_SOURCE ${PROJECT_SOURCE_DIR}/src/*.cpp)
# 这里 SOURCE 表示所有用于编译成库的 cpp 文件
set(SOURCE ${CXX_SOURCE})



## Your package locations should be listed before other locations
include_directories(
		include
		/opt/Baltamatica/include
    /opt/Baltamatica/include/bex
    /opt/Baltamatica/include/ba_obj
)



if (WIN32)
	#我的libbex.dll.a libbaltam_core.dll.a 都在 /usr/local/lib 下
	link_libraries(-L /opt/Baltamatica/lib -lbex -lbt_core)
endif (WIN32)

add_library(${PROJECT_NAME}  SHARED ${SOURCE})

# 插件的文件名总是 main
set_target_properties(${PROJECT_NAME}  PROPERTIES OUTPUT_NAME "main")

# module 类型的库不需要 lib 前缀
set_target_properties(${PROJECT_NAME}  PROPERTIES PREFIX "")

## 链接bex库
if(WIN32)
    message("-->> windows link library")
else()
    message("-->> Linux link library")
    ## Find catkin macros and libraries
    find_library(BEX bex )
		message(${BEX})
    target_link_libraries(${PROJECT_NAME} ${BEX})
endif()
# BLAS/LAPACK
target_link_libraries(${PROJECT_NAME} -L/opt/Baltamatica/lib/ -lopenblas ) 




