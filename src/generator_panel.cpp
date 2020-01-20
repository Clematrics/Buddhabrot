#include "generator_panel.h"

#include "generator_info.h"
#include "image.h"

#include "glad/glad.h"
#include "imgui.h"

const GLchar* vertex_source { R"glsl(
	#version 330 core
	in vec2 position;
	in vec2 texcoord;
	out vec2 Texcoord;
	uniform vec2 window_size;
	void main()
	{
		Texcoord = texcoord;
		if (window_size.x > window_size.y) {
			float x = position.x * window_size.y / window_size.x;
			gl_Position = vec4(x, position.y, 0.0, 1.0);
		}
		else {
			float y = position.y * window_size.x / window_size.y;
			gl_Position = vec4(position.x, y, 0.0, 1.0);
		}
	}
)glsl" };

const GLchar* fragment_source { R"glsl(
	#version 330 core
	in vec2 Texcoord;
	out vec4 outColor;
	uniform sampler2D buddhabrot;
	void main()
	{
		outColor = texture(buddhabrot, Texcoord);
	}
)glsl" };

generator_panel::generator_panel()
:   image_ptr(std::make_shared<image>(properties.image_width, properties.image_height))
,   gen_ptr(std::make_unique<generator>(image_ptr, properties, parameters, runtime_parameters))
{
	// Create a Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create a Vertex Buffer Object and copy the vertex data to it
	glGenBuffers(1, &vbo);

	vertices = {
	//  Position      Texcoords
		-1.0f,  1.0f, 0.0f, 1.0f, // Top-left
		 1.0f,  1.0f, 0.0f, 0.0f, // Top-right
		 1.0f, -1.0f, 1.0f, 0.0f, // Bottom-right
		-1.0f, -1.0f, 1.0f, 1.0f  // Bottom-left
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

	// Create an element array
	glGenBuffers(1, &ebo);

	elements = {
		0, 1, 2,
		2, 3, 0
	};

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(GLuint), elements.data(), GL_STATIC_DRAW);

	// Create and compile the vertex shader
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_source, NULL);
	glCompileShader(vertex_shader);

	// Create and compile the fragment shader
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_source, NULL);
	glCompileShader(fragment_shader);

	// Link the vertex and fragment shader into a shader program
	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glBindFragDataLocation(shader_program, 0, "outColor");
	glLinkProgram(shader_program);
	glUseProgram(shader_program);

	// Specify the layout of the vertex data
	GLint pos_attrib = glGetAttribLocation(shader_program, "position");
	glEnableVertexAttribArray(pos_attrib);
	glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	GLint tex_attrib = glGetAttribLocation(shader_program, "texcoord");
	glEnableVertexAttribArray(tex_attrib);
	glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	// Load textures
	glGenTextures(1, &texture);
}

generator_panel::~generator_panel() {
	glDeleteTextures(1, &texture);

	glDeleteProgram(shader_program);
	glDeleteShader(fragment_shader);
	glDeleteShader(vertex_shader);

	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);

	glDeleteVertexArrays(1, &vao);
}

void generator_panel::display(int display_width, int display_height) {
	display_panel();
	display_image(display_width, display_height);
}

void generator_panel::display_panel() {
	ImGui::Begin("Buddhabrot generator");

	if (ImGui::CollapsingHeader("New")) {
		ImGui::InputScalar("Image width", ImGuiDataType_U16, &properties.image_width);
		ImGui::InputScalar("Image height", ImGuiDataType_U16, &properties.image_height);

		double a_real { properties.corner_a.real() }
		     , a_imag { properties.corner_a.imag() }
		     , b_real { properties.corner_b.real() }
		     , b_imag { properties.corner_b.imag() };
		ImGui::InputScalar("Corner A (real)", ImGuiDataType_Double, &a_real);
		ImGui::InputScalar("Corner A (imag)", ImGuiDataType_Double, &a_imag);
		ImGui::InputScalar("Corner B (real)", ImGuiDataType_Double, &b_real);
		ImGui::InputScalar("Corner B (imag)", ImGuiDataType_Double, &b_imag);
		properties.corner_a.real(a_real);
		properties.corner_a.imag(a_imag);
		properties.corner_b.real(b_real);
		properties.corner_b.imag(b_imag);

		if ((gen_ptr->get_status() == status::Stopped)
		&& ImGui::Button("New generator")) {
			image_ptr = std::make_shared<image>(properties.image_width, properties.image_height);
			gen_ptr = std::make_unique<generator>(image_ptr, properties, parameters, runtime_parameters);
		}
	}
	// if (ImGui::CollapsingHeader("Load")) {}
	// if (ImGui::CollapsingHeader("Save")) {}
	// if (ImGui::CollapsingHeader("Picture rendering")) {}
	if (ImGui::CollapsingHeader("Sequence parameters")) {
		ImGui::InputScalar("Escape norm (squared)", ImGuiDataType_Double, &parameters.escape_norm);
		ImGui::InputScalar("Iterations to escape", ImGuiDataType_U64, &parameters.iterations_to_escape);

		ImGui::Checkbox("Y symetry", &parameters.y_symetry);

		if ((gen_ptr->get_status() == status::Stopped)
		&& ImGui::Button("Set sequence parameters")) {
			gen_ptr->set_parameters(parameters);
		}
	}
	if (ImGui::CollapsingHeader("Runtime parameters")) {
		ImGui::InputScalar("Threads in pool", ImGuiDataType_U16, &runtime_parameters.threads_number);
		ImGui::InputScalar("Total of points", ImGuiDataType_U64, &runtime_parameters.points_target);
		ImGui::InputScalar("Points in pool", ImGuiDataType_U64, &runtime_parameters.pool_batch_size);
		ImGui::InputScalar("Points in batch", ImGuiDataType_U64, &runtime_parameters.thread_batch_size);

		if ((gen_ptr->get_status() == status::Stopped)
		&& ImGui::Button("Set runtime parameters")) {
			gen_ptr->set_runtime_parameters(runtime_parameters);
		}
	}
	if (ImGui::CollapsingHeader("Runtime control")) {
		std::string_view status_string { status_to_string(gen_ptr->get_status()) };
		ImGui::Text("Generator's status : %s", status_string.data());

		if (ImGui::Button("Resume"))       { gen_ptr->resume(); }
		if (ImGui::Button("Pause"))        { gen_ptr->pause(); }
		if (ImGui::Button("Finish batch")) { gen_ptr->finish_bash(); }
		if (ImGui::Button("Stop"))         { gen_ptr->stop(); }

		for (auto& [progress, batch_size] : gen_ptr->progress()) {
			ImGui::Text("%lu / %lu", progress, batch_size);
			ImGui::SameLine(150);
			ImGui::ProgressBar((float) progress / batch_size);
		}
		auto [pool_progress, pool_size] = gen_ptr->pool_progress();
		if (pool_size != 0) {
			ImGui::Text("Pool : %lu / %lu", pool_progress, pool_size);
			ImGui::SameLine(300);
			ImGui::ProgressBar((float) pool_progress / pool_size);
		}
		else {
			ImGui::Text("Pool : %lu", pool_progress);
		}
		auto [total_progress, total] = gen_ptr->total_progress();
		if (total != 0) {
			ImGui::Text("Total : %lu / %lu", total_progress, total);
			ImGui::SameLine(300);
			ImGui::ProgressBar((float) total_progress / total);
		}
		else {
			ImGui::Text("Total : %lu", total_progress);
		}
	}

	ImGui::End();
}

void generator_panel::display_image(int display_width, int display_height) {
	glUseProgram(shader_program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	std::vector<pixel> image = image_ptr->get_image();
	uint8_t* pixels_ptr = reinterpret_cast<uint8_t*>(image.data());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gen_ptr->properties.image_width, gen_ptr->properties.image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels_ptr);
	glUniform1i(glGetUniformLocation(shader_program, "buddhabrot"), 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glUniform2f(glGetUniformLocation(shader_program, "window_size"), (float)display_width, (float)display_height);

	// Draw a rectangle from the 2 triangles using 6 indices
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	// glDrawArrays(GL_TRIANGLES, 0, 6);
}

uint16_t generator_panel::imageWidth() {
	return image_ptr->width();
}

uint16_t generator_panel::imageHeight() {
	return image_ptr->height();
}