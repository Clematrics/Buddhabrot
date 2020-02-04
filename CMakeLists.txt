cmake_minimum_required(VERSION 3.10)

project(Buddhabrot)

find_package(glfw3 CONFIG REQUIRED)

add_subdirectory(dependencies/imgui)
add_subdirectory(dependencies/glad)

include_directories(
	include
)

set(
	sources
	src/generator_info.cpp
	src/generator_panel.cpp
	src/generator.cpp
	src/image.cpp
	src/imgui_impl_glfw.cpp
	src/imgui_impl_opengl3.cpp
	src/main.cpp
	src/monte_carlo_sampler.cpp
	src/monte_carlo_tree.cpp
	src/uniform_sampler.cpp
)

set(
	headers
	include/abstract_image.h
	include/compressed_image.h
	include/generator_info.h
	include/generator_panel.h
	include/generator.h
	include/helper.h
	include/image.h
	include/image_converter.h
	include/imgui_impl_glfw.h
	include/imgui_impl_opengl3.h
	include/mandelbrot_helper.h
	include/monte_carlo_sampler.h
	include/monte_carlo_tree.h
	include/sampler.h
	include/types.h
	include/uniform_sampler.h
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# set(CMAKE_CXX_COMPILER clang-8)

add_executable(${PROJECT_NAME} ${sources} ${headers})
# target_compile_options(${PROJECT_NAME} PUBLIC "-ggdb")
target_compile_options(${PROJECT_NAME} PUBLIC "-O2")
# target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>")
target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
target_link_libraries(
	${PROJECT_NAME}
	PUBLIC
	$<$<PLATFORM_ID:Linux>:pthread>
	imgui
	glad
	glfw
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)