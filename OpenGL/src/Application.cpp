#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))

static void GLClearError() {
	while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line) {
	while (GLenum error = glGetError()) {
		std::cout << "[OpenGL Error] (" << error << "): " << function <<
			" " << file << ":" << line << std::endl;
		return false;
	}
	return true;
}

struct ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath) {
	// NOTE: using modern C++ here BUT old-school C is faster (better for Game Engines)
	std::ifstream stream(filepath);

	enum class ShaderType {
		NONE = -1,
		VERTEX = 0,
		FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;

	// check if we are reading vertex or fragment shader code
	// append each line to the corresponding stringstream ([0]: vertex, [1]: fragment)
	while (getline(stream, line)) {
		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos) {
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos) {
				type = ShaderType::FRAGMENT;
			}
			
		}
		else {
			ss[(int)type] << line << '\n';
		}
	}

	return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source) {
	GLCall(unsigned int id = glCreateShader(type));
	const char* src = source.c_str();
	GLCall(glShaderSource(id, 1, &src, nullptr));	// legnth = nullptr to read the whole source code
	GLCall(glCompileShader(id));

	// Error handling
	int result;
	GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE) {
		int length;
		GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)alloca(length * sizeof(char));
		GLCall(glGetShaderInfoLog(id, length, &length, message));
		std::cout << "Failed to compile" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader!" << std::endl;
		std::cout << message << std::endl;
		GLCall(glDeleteShader(id));
		return 0;
	}

	return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
	// NOTE: using unsigned int instead of GL_UINT makes it more generic and helps with multiple APIs (Vulkan, DirectX)
	GLCall(unsigned int program = glCreateProgram());
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	// Link shaders into one program
	glAttachShader(program, vs);	// FIXME: this throws an error for some reason when wrapped with GLCall()
	GLCall(glAttachShader(program, fs));
	GLCall(glLinkProgram(program));
	GLCall(glValidateProgram(program));

	GLCall(glDeleteShader(vs));
	GLCall(glDeleteShader(fs));

	return program;
}

int main(void)
{
	GLFWwindow* window;

	/* 
	Initialize the library */
	if (!glfwInit())
		return -1;

	// create context with core profile (OpenGL 3.3)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	// synchronize without vsync
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
		std::cout << "Error!" << std::endl;

	GLCall(std::cout << glGetString(GL_VERSION) << std::endl);

	// Define data (triangle) for shader
	float positions[] = {
		-0.5f, -0.5f,
		0.5f, -0.5f,
		0.5f, 0.5f,
		-0.5f, 0.5f,
	};

	// index buffer: instead of adding all vertices inside positions, add only the unique ones
	// and tell OpenGL which ones to draw in which order
	// draw square => 2 triangles
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0,
	};

	// vertex array object (specifies layout of vertex buffer, i.e. start at 0 with vertex length = 2)
	unsigned int vao;
	GLCall(glGenVertexArrays(1, &vao));
	GLCall(glBindVertexArray(vao));

	// vertex buffer
	unsigned int buffer;
	GLCall(glGenBuffers(1, &buffer));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
	GLCall(glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW));

	GLCall(glEnableVertexAttribArray(0));
	GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

	// index buffer
	unsigned int ibo;	// index buffer object
	GLCall(glGenBuffers(1, &ibo));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW));

	// vertex shader: where to draw	(outline, positions)
	// fragment shader: what color to draw (pixels)
	ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
	unsigned int shader = CreateShader(source.FragmentSource, source.FragmentSource);
	GLCall(glUseProgram(shader));

	GLCall(int location = glGetUniformLocation(shader, "u_Color"));
	ASSERT(location != -1);
	GLCall(glUniform4f(location, 0.8f, 0.3f, 0.8f, 1.0f));

	// unbind everything
	GLCall(glBindVertexArray(0));
	GLCall(glUseProgram(0));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

	float r = 0.0f;
	float increment = 0.05f;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		GLCall(glClear(GL_COLOR_BUFFER_BIT));

		// bind shader
		GLCall(glUseProgram(shader));
		GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));

		GLCall(glBindVertexArray(vao));	// bind vertex array
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));	// bind index buffer

		// draw
		GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

		if (r > 1.0f) {
			increment = -0.05f;
		}
		else if (r < 0.0f) {
			increment = 0.05;
		}
		r += increment;

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	GLCall(glDeleteProgram(shader));

	glfwTerminate();
	return 0;
}
