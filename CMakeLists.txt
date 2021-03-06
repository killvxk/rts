﻿cmake_minimum_required (VERSION 3.6)
set(CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /Wall /GR /EHsc" CACHE INTERNAL "")

include_directories(error socks common memory)
file(GLOB_RECURSE RTS_SOURCES "*.c*")

if (WIN32)
	include_directories(thirdparty/pthread/include)

	find_library(WSOCK32_LIBRARY wsock32)
	find_library(WS2_32_LIBRARY ws2_32)
	set(SOCKET_LIBRARIES ${WSOCK32_LIBRARY} ${WS2_32_LIBRARY})	
	get_filename_component(PTHREAD_LIBRARIES "thirdparty/pthread/pthreadVC2.lib" ABSOLUTE)

	file(COPY "thirdparty/pthread/pthreadVC2.dll" DESTINATION ${CMAKE_BINARY_DIR})
endif()



add_executable (rts ${RTS_SOURCES})
target_link_libraries(rts ${SOCKET_LIBRARIES} ${PTHREAD_LIBRARIES})