cmake_minimum_required (VERSION 3.10)
project (imgui)

set(IMGUI_HEADERS
	imgui/imconfig.h
	imgui/imgui.h
	imgui/imgui_internal.h
	imgui/imstb_rectpack.h
	imgui/imstb_textedit.h
	imgui/imstb_truetype.h
)
include_directories(imgui)

set(IMGUI_SOURCES
	imgui/imgui.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_widgets.cpp
	imgui/imgui_demo.cpp
)

add_library(${PROJECT_NAME} STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC imgui)