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
	src/generator/generator_info.cpp
	src/generator/generator.cpp

	src/gui/generator_panel.cpp

	src/image/image.cpp

	src/imgui/imgui_impl_glfw.cpp
	src/imgui/imgui_impl_opengl3.cpp

	src/sampler/monte_carlo_sampler.cpp
	src/sampler/monte_carlo_tree.cpp
	src/sampler/uniform_sampler.cpp

	src/main.cpp
)

set(
	headers
	include/generator/generator_info.h
	include/generator/generator.h

	include/gui/generator_panel.h

	include/image/abstract_image.h
	include/image/compressed_image.h
	include/image/image_converter.h
	include/image/image.h

	include/imgui/imgui_impl_glfw.h
	include/imgui/imgui_impl_opengl3.h

	include/sampler/monte_carlo_sampler.h
	include/sampler/monte_carlo_tree.h
	include/sampler/sampler.h
	include/sampler/uniform_sampler.h

	include/helper.h
	include/mandelbrot_helper.h
	include/types.h
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