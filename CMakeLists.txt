cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(OpenGL_GL_PREFERENCE "LEGACY")
set(CMAKE_VERBOSE_MAKEFILE:BOOL "ON")
#SET(CMAKE_CONFIGURATION_TYPES "RelWithDebInfo" CACHE STRING "" FORCE)

project(main)
add_definitions(-w)

aux_source_directory(./src main_srcs)


file(GLOB static_libraries  "${CMAKE_SOURCE_DIR}/depend/lib/*.lib")
file(GLOB shared_libraries  "${CMAKE_SOURCE_DIR}/depend/lib/*.dll")
include_directories(${CMAKE_SOURCE_DIR}/depend/include)

add_executable(main ${main_srcs})
target_link_libraries(main PRIVATE ${static_libraries} )


#add_custom_command(
#	TARGET main POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/patch" $<TARGET_FILE_DIR:main>
#)
#add_custom_command(
#		TARGET main POST_BUILD
#		COMMAND ${CMAKE_COMMAND} -E copy_directory "${shared_libraries}" $<TARGET_FILE_DIR:main>
#)


