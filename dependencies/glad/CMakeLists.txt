cmake_minimum_required(VERSION 3.10)
project(glad)

set(GLAD_SOURCES
	src/gl.c
)

include_directories(include)
add_library(${PROJECT_NAME} STATIC ${GLAD_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)