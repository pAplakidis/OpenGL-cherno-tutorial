#pragma once

#include <string>
#include <unordered_map>

struct ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
};

// vertex shader: where to draw	(outline, positions)
// fragment shader: what color to draw (pixels)
class Shader {
private:
	std::string m_FilePath;
	unsigned int m_RendererID;
	std::unordered_map<std::string, int> m_UniformLocationCache;	//caching for uniforms

public:
	Shader(const std::string& filepath);
	~Shader();

	void Bind() const;
	void Unbind() const;

	// Set uniforms
	void SetUnfiform1i(const std::string& name, int value);
	void SetUnfiform1f(const std::string& name, float value);
	void SetUnfiform4f(const std::string& name, float v0, float v1, float f2, float f3);

private:
	ShaderProgramSource ParseShader(const std::string& filepath);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);

	int GetUniformLocation(const std::string& name);
};
