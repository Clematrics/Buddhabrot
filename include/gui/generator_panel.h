#pragma once

#include "generator/generator.h"
#include "generator/generator_info.h"
#include "image/abstract_image.h"
#include "types.h"

#include "glad/glad.h"

constexpr uint16_t IMG_WIDTH = 720;
constexpr uint16_t IMG_HEIGHT = 720;

class generator_panel {
public:
	generator_panel();
	~generator_panel();

	void display(int display_width, int display_height);
	uint16_t imageWidth();
	uint16_t imageHeight();
private:
	void display_panel();
	void display_image(int display_width, int display_height);

	generator_properties properties;
	generator_parameters parameters;
	generator_runtime_parameters runtime_parameters;
	std::shared_ptr<abstractImage> image_ptr;
	std::unique_ptr<generator> gen_ptr;

	// OpenGL info for rendering the image
	GLuint vao;
	GLuint vbo;
	std::vector<GLfloat> vertices;
	GLuint ebo;
	std::vector<GLuint> elements;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint shader_program;
	GLuint texture;
};