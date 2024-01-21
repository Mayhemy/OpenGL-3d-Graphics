#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>

inline std::string ReadTextFromFile(std::string path) {
	std::ifstream file(path);
	std::string str;
	std::string line;
	while (std::getline(file, line)) {
		str += line + "\n";
	}
	return str;
}

struct Shader {

	int ID = -1;

	void Load(std::string vertexPath, std::string fragmentPath) {


		// ucitavamo kod iz vertex shadera i fragment shadera
		std::string vertexSource = ReadTextFromFile("res/shaders/" + vertexPath);
		std::string fragmentSource = ReadTextFromFile("res/shaders/" + fragmentPath);

		// OPENGL bs 
		const char* vShaderCode = vertexSource.c_str();
		const char* fShaderCode = fragmentSource.c_str();

		// pravimo shader, stavljamo da je source vertex shadercode i compilujemo
		unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);

		// napravimo fragment, source fragment shadercode i compile
		unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);

		// program object pravimo koji vraca id (container za shadere)	
		ID = glCreateProgram();
		// ovde ih povezujemo za dobijeni ID
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		// ovde sve linkujemo u finalni shader program (outputi iz vertexa idu u inpute fragmenta
		glLinkProgram(ID);
		// clearujemo shadere
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	void Bind() {
		glUseProgram(ID);
	}
	// uzmemo lokaciju sa idjem i imenom za ovaj shader i nasetujemo na value
	void SetMat4(const std::string& name, glm::mat4 value) {
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
	}

	void SetVec3(const std::string& name, const glm::vec3& value) {
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
};