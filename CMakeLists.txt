cmake_minimum_required(VERSION 3.10)

project(Buddhabrot)

find_package(glfw3 REQUIRED)

add_subdirectory(dependencies/imgui)
add_subdirectory(dependencies/glad)

include_directories(
	include
)

file(
	GLOB
	sources
	src/*.cpp
)

add_executable(${PROJECT_NAME} ${sources})
target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
	imgui
	# imgui_demo
	glad
	glfw
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

install(TARGETS Buddhabrot DESTINATION bin)