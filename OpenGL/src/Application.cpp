#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
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

	// create scope to avoid errors with glfwTerminate() due to context (alternatively heap allocate Vertex and Index Buffers)
	{
		// Define data (triangle) for shader
		float positions[] = {
			-0.5f, -0.5f, 0.0f, 0.0f,	// 0
			0.5f, -0.5f, 1.0f, 0.0f,	// 1
			0.5f, 0.5f,	1.0f, 1.0f,		// 2
			-0.5f, 0.5f, 0.0f, 1.0f		// 3
		};

		// index buffer: instead of adding all vertices inside positions, add only the unique ones
		// and tell OpenGL which ones to draw in which order
		// draw square => 2 triangles
		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0,
		};

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		// vertex array object (specifies layout of vertex buffer, i.e. start at 0 with vertex length = 2)
		unsigned int vao;
		GLCall(glGenVertexArrays(1, &vao));
		GLCall(glBindVertexArray(vao));

		VertexArray va;
		VertexBuffer vb(positions, 4 * 4 * sizeof(float));
		VertexBufferLayout layout;
		layout.Push<float>(2);
		layout.Push<float>(2);
		va.AddBuffer(vb, layout);

		IndexBuffer ib(indices, 6);

		Shader shader("res/shaders/Basic.shader");
		shader.Bind();
		shader.SetUnfiform4f("u_Color", 0.8f, 0.3f, 0.8f, 1.0f);

		Texture texture("res/textures/spiderman.png");
		texture.Bind();
		shader.SetUnfiform1i("u_Texture", 0);

		// unbind everything
		va.Unbind();
		shader.Unbind();
		vb.Unbind();
		ib.Unbind();

		Renderer renderer;

		float r = 0.0f;
		float increment = 0.05f;

		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			renderer.Clear();

			shader.Bind();
			shader.SetUnfiform4f("u_Color", r, 0.3f, 0.8f, 1.0f);

			renderer.Draw(va, ib, shader);

			// draw
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
	}

	glfwTerminate();
	return 0;
}
