#pragma once

//Includes
#include <string>
#include <fstream>
#include <vector>
#include "../OpenGL/OpenGLInc.h"
#include "../../Maths/Maths.h"

namespace UnifiedEngine {
	class Shader {
	private:
		//Ids
		GLuint ProgramID;

		//OpenGL
		const int versionMajor;
		const int versionMinor;

		//Load
		GLuint loadShader(GLenum type, const char* shaderData) {
			char infoLog[512];
			GLint success;

			//Create a shader
			GLuint shader = glCreateShader(type);

			//Load the Src
			string str_src = shaderData;
			const GLchar* Src = str_src.c_str();

			//Compile the shader
			glShaderSource(shader, 1, &Src, NULL);
			glCompileShader(shader);

			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shader, 512, NULL, infoLog);
			}

			//Return the shader id
			return shader;
		}

		//Link
		void linkProgram(GLuint vertexShader, GLuint fragmentShader) {
			char infoLog[512];
			GLint success;

			//Create the program
			this->ProgramID = glCreateProgram();

			//Bind the program to shaders
			glAttachShader(this->ProgramID, vertexShader);

			glAttachShader(this->ProgramID, fragmentShader);

			//Link the program
			glLinkProgram(this->ProgramID);

			glGetProgramiv(this->ProgramID, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(this->ProgramID, 512, NULL, infoLog);
			}

			//Unbind
			glUseProgram(0);
		}

	public:
		const char* Name;

		//Con/Destructors
		Shader(const char* vertexShaderData, const char* fragmentShaderData)
			: versionMajor(GLMajor), versionMinor(GLMinor)
		{
			//Set Defaults
			GLuint vertexShader = 0;
			GLuint geometryShader = 0;
			GLuint fragmentShader = 0;

			//Load Specific shaders
			vertexShader = this->loadShader(GL_VERTEX_SHADER, vertexShaderData);

			fragmentShader = this->loadShader(GL_FRAGMENT_SHADER, fragmentShaderData);

			//Link Them
			this->linkProgram(vertexShader, fragmentShader);

			//Delete them
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			//Unbind
			glUseProgram(0);
		}

		~Shader() {
			//Delete the program
			glDeleteProgram(this->ProgramID);
		}

		//Get the Program
		inline GLuint getProgram() const {
			return this->ProgramID;
		}

		//Linker
		void use() {
			//Bind shader
			glUseProgram(this->ProgramID);
		}

		//Unlink
		void unbind() {
			//Unbinds the shader
			glUseProgram(0);
		}

		//Setting a integer to a unifrom in the shader
		void set1i(GLint value, const GLchar* name)
		{
			this->use();

			//Sets a integer
			glUniform1i(glGetUniformLocation(this->ProgramID, name), value);

			this->unbind();
		}

		//Setting a float to a unifrom in the shader
		void set1f(GLfloat value, const GLchar* name)
		{
			this->use();

			//Sets a float
			glUniform1f(glGetUniformLocation(this->ProgramID, name), value);

			this->unbind();
		}

		//Setting a vec2 to a unifrom in the shader
		void setVec2f(fvec2 value, const GLchar* name)
		{
			this->use();

			//Sets a vec2
			glUniform2fv(glGetUniformLocation(this->ProgramID, name), 1, value_ptr(value));

			this->unbind();
		}

		//Setting a vec3 to a unifrom in the shader
		void setVec3f(fvec3 value, const GLchar* name)
		{
			this->use();

			//Sets a vec3
			glUniform3fv(glGetUniformLocation(this->ProgramID, name), 1, value_ptr(value));

			this->unbind();
		}

		//Setting a vec4 to a unifrom in the shader
		void setVec4f(fvec4 value, const GLchar* name)
		{
			this->use();

			//Sets a vec4
			glUniform4fv(glGetUniformLocation(this->ProgramID, name), 1, value_ptr(value));

			this->unbind();
		}

		//Setting a mat3 to a unifrom in the shader
		void setMat3fv(mat3 value, const GLchar* name, GLboolean transpose = GL_FALSE)
		{
			this->use();

			//Sets a mat3
			glUniformMatrix3fv(glGetUniformLocation(this->ProgramID, name), 1, transpose, value_ptr(value));

			this->unbind();
		}

		//Setting a mat4 to a unifrom in the shader
		void setMat4fv(mat4 value, const GLchar* name, GLboolean transpose = GL_FALSE)
		{
			this->use();

			//Sets a mat4
			glUniformMatrix4fv(glGetUniformLocation(this->ProgramID, name), 1, transpose, value_ptr(value));

			this->unbind();
		}
	};
}
