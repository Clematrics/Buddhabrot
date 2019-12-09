#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "glad/gl.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <string>

// settings
// --------
constexpr unsigned int SCR_WIDTH = 1280;
constexpr unsigned int SCR_HEIGHT = 720;

constexpr unsigned int IMG_WIDTH = 720;
constexpr unsigned int IMG_HEIGHT = 720;

#include "generator.h"

static generator gen(IMG_WIDTH, IMG_HEIGHT);

void displayControlwindow() {

	ImGui::Begin("Buddhabrot generator");

	if (ImGui::CollapsingHeader("New")) {

	}

	if (ImGui::CollapsingHeader("Load")) {

	}

	if (ImGui::CollapsingHeader("Save")) {

	}

	if (ImGui::CollapsingHeader("Picture")) {

	}

	if (ImGui::CollapsingHeader("Parameters")) {

	}

	if (ImGui::CollapsingHeader("Runtime info")) {
		if (ImGui::Button("Play"))
			gen.m_status = status::Running;
		if (ImGui::Button("Stop"))
			gen.m_status = status::Stopping;

		ImGui::Text("runtime_total_points : %lu", gen.runtime_total_points);
		ImGui::Text("runtime_batch_points : %lu", gen.runtime_batch_points);

		for (auto& progress : gen.threads_progress) {
			// ImGui::ProgressBar(progress);
			ImGui::Text("%lu", progress);
		}
	}

	ImGui::End();
}

// Shader sources
const GLchar* vertexSource = R"glsl(
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
)glsl";
const GLchar* fragmentSource = R"glsl(
	#version 330 core
	in vec2 Texcoord;
	out vec4 outColor;
	uniform sampler2D buddhabrot;
	void main()
	{
		outColor = texture(buddhabrot, Texcoord);
	}
)glsl";


int main(int argc, char** argv) {

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	const char* glsl_version = "#version 330";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Buddhabrot generator", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGL(glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Setup Dear ImGui context
	// ------------------------
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

	// Setup Dear ImGui style
	// ----------------------
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	// --------------------------------
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Our state
	bool show_demo_window = true;
	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

	// Setup OpenGL data to display the reesult image
	// ----------------------------------------------

	// Create a Vertex Array Object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create a Vertex Buffer Object and copy the vertex data to it
	GLuint vbo;
	glGenBuffers(1, &vbo);

	GLfloat vertices[] = {
	//  Position      Texcoords
		-1.0f,  1.0f, 0.0f, 1.0f, // Top-left
		 1.0f,  1.0f, 0.0f, 0.0f, // Top-right
		 1.0f, -1.0f, 1.0f, 0.0f, // Bottom-right
		-1.0f, -1.0f, 1.0f, 1.0f  // Bottom-left
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Create an element array
	GLuint ebo;
	glGenBuffers(1, &ebo);

	GLuint elements[] = {
		0, 1, 2,
		2, 3, 0
	};

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	// Create and compile the vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	// Create and compile the fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	// Link the vertex and fragment shader into a shader program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	// Specify the layout of the vertex data
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	// Load textures
	GLuint texture;
	glGenTextures(1, &texture);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)) {
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		displayControlwindow();

		glUseProgram(shaderProgram);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		std::vector<pixel> image = gen.image_ptr->getImage();
		uint8_t* image_ptr = reinterpret_cast<uint8_t*>(image.data());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, IMG_WIDTH, IMG_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, image_ptr);
		glUniform1i(glGetUniformLocation(shaderProgram, "buddhabrot"), 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glUniform2f(glGetUniformLocation(shaderProgram, "window_size"), (float)display_w, (float)display_h);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		// {
		// 	static float f = 0.0f;
		// 	static int counter = 0;

		// 	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		// 	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		// 	ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

		// 	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		// 	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		// 	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		// 		counter++;
		// 	ImGui::SameLine();
		// 	ImGui::Text("counter = %d", counter);

		// 	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		// 	ImGui::End();
		// }

		// Rendering
		ImGui::Render();
		// Clear the screen to black
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		// Draw a rectangle from the 2 triangles using 6 indices
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	gen.m_status = status::Stopping;

	// Cleanup OpenGL
	// --------------
	glDeleteTextures(1, &texture);

	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);

	glDeleteVertexArrays(1, &vao);

	// Cleanup ImGui
	// -------------
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
