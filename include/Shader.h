#ifndef SHADER_H
#define	SHADER_H
#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/glm.hpp>
#include<glm/gtc/type_ptr.hpp>
using namespace std;
enum class ShaderTYPE
{
	VERTEX,
	//	GEOMETRY,
	FRAGMENT,
	COMPUTE,
	PROGRAM,
	SIMPLE
};
class Shader
{
private:
	std::string filename;
	static string read_file(string filename)
	{
		ifstream read(filename);
		stringstream ss;
		ss << read.rdbuf();
		return ss.str();
	}
	
	static void checkProg(unsigned int prog, ShaderTYPE type)
	{
		int success;
		char infolog[512];
		switch (type) 
		{
			case ShaderTYPE::VERTEX:
			case ShaderTYPE::FRAGMENT:
			case ShaderTYPE::COMPUTE:
			{
				
				glGetShaderiv(prog, GL_COMPILE_STATUS, &success);
				if (!success)
				{
					glGetShaderInfoLog(prog, 512, NULL, infolog);
					switch (type)
					{
						case ShaderTYPE::VERTEX:
						{
#ifndef __HIDE_SHADER_LOGS__
							cout << "An error happend while compilation of vertex shader: " << infolog << endl;
#endif
							break;
						}
						case ShaderTYPE::FRAGMENT:
						{
#ifndef __HIDE_SHADER_LOGS__
							cout << "An error happend while compilation of fragment shader: " << infolog << endl;
#endif
							break;
						}
						/*case GEOMETRY:
						{
						#ifndef __HIDE_SHADER_LOGS__
							cout << "An error happend while compilation of geometry shader: " << infolog << endl;
						#endif
							break;
						}*/
						case ShaderTYPE::COMPUTE:
						{
#ifndef __HIDE_SHADER_LOGS__
							cout << "An error happend while compilation of compute shader: " << infolog << endl;
#endif
							break;
						}

					}
					return;
				}
				break;
			}
			case ShaderTYPE::PROGRAM:
			{
				glGetProgramiv(prog, GL_LINK_STATUS, &success);
				if (!success)
				{
					glGetProgramInfoLog(prog, 512, NULL, infolog);
#ifndef __HIDE_SHADER_LOGS__
					cout << "An error happend while linking shader program: " << infolog << endl;
#endif
					return;
				}
				break;
			}
		}

	}
public:
	unsigned int ID;
	unsigned int computeID = 0;
	Shader(string vertex_file, string fragment_file, string geometry_file = "")
	{
		filename = vertex_file.substr(0, vertex_file.find_first_of('_', 0));
		string vertex_str = read_file(vertex_file);
		string fragment_str = read_file(fragment_file);
		const char* vertex_shader_source = vertex_str.c_str();
		const char* fragment_shader_source = fragment_str.c_str();
		unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
		glCompileShader(vertex_shader);
		checkProg(vertex_shader, ShaderTYPE::VERTEX);
		unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
		glCompileShader(fragment_shader);
		checkProg(fragment_shader, ShaderTYPE::FRAGMENT);
		
		unsigned int shader_program = glCreateProgram();
		glAttachShader(shader_program, vertex_shader);
		glAttachShader(shader_program, fragment_shader);
		/*if (geometry_file != "")
		{
			string geometry_shd = read_file(geometry_file);
			unsigned int geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
			const char* geometry_shader_source = geometry_shd.c_str();
			glShaderSource(geometry_shader, 1, &geometry_shader_source, NULL);
			glCompileShader(geometry_shader);
			checkProg(geometry_shader, GEOMETRY);
			glAttachShader(shader_program, geometry_shader);
			glDeleteShader(geometry_shader);
		}*/
		glLinkProgram(shader_program);
		checkProg(shader_program, ShaderTYPE::PROGRAM);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		ID = shader_program;
	}
	~Shader()
	{
		glDeleteProgram(ID);
		if (computeID) glDeleteProgram(computeID);
	}
	void use() const
	{
		glUseProgram(ID);
	}
	void createComputeShader(string compute_file)
	{
		string compute_str = read_file(compute_file);
		const char* compute_shader_source = compute_str.c_str();
		unsigned int compute_shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(compute_shader, 1, &compute_shader_source, NULL);
		glCompileShader(compute_shader);
		checkProg(compute_shader, ShaderTYPE::COMPUTE);
		computeID = glCreateProgram();
		glAttachShader(computeID, compute_shader);
		glLinkProgram(computeID);
		checkProg(computeID, ShaderTYPE::PROGRAM);
		glDeleteShader(compute_shader);
	}
	void SetInt(string name, int value, ShaderTYPE type = ShaderTYPE::SIMPLE)
	{
		unsigned int usedID;
		if (type == ShaderTYPE::SIMPLE) usedID = ID;
		else usedID = computeID;
		int loc = glGetUniformLocation(usedID, name.c_str());
		if (loc == -1)
		{
#ifndef __HIDE_SHADER_LOGS__
			cout << "Didn't find uniform int " << name << " in shader prog" << filename << endl;
#endif 
			return;
		}

		glUniform1i(loc, value);
	}
	void SetInt64(string name, GLuint64 value, ShaderTYPE type = ShaderTYPE::SIMPLE)
	{
		unsigned int usedID;
		if (type == ShaderTYPE::SIMPLE) usedID = ID;
		else usedID = computeID;
		int loc = glGetUniformLocation(usedID, name.c_str());
		if (loc == -1)
		{
#ifndef __HIDE_SHADER_LOGS__
			cout << "Didn't find uniform int " << name << " in shader prog" << filename << endl;
#endif 
			return;
		}
		glUniform1ui64ARB(loc, value);
	}
	void SetFloat(string name, float value, ShaderTYPE type = ShaderTYPE::SIMPLE)
	{
		unsigned int usedID;
		if (type == ShaderTYPE::SIMPLE) usedID = ID;
		else usedID = computeID;
		int loc = glGetUniformLocation(usedID, name.c_str());
		if (loc == -1)
		{
#ifndef __HIDE_SHADER_LOGS__
			cout << "Didn't find uniform float " << name << " in shader prog" << filename << endl;
#endif 
			return;
		}
		glUniform1f(loc, value);
	}
	void Set2UInt(string name, GLuint64 val, ShaderTYPE type = ShaderTYPE::SIMPLE)
	{
		unsigned int usedID;
		if (type == ShaderTYPE::SIMPLE) usedID = ID;
		else usedID = computeID;
		int loc = glGetUniformLocation(usedID, name.c_str());
		if (loc == -1)
		{
#ifndef __HIDE_SHADER_LOGS__
			cout << "Didn't find uniform 2UI " << name << " in shader prog" << filename << endl;
#endif
			return;
		}
		glUniform2ui(loc, (GLuint)(val & 0xFFFFFFFF), (GLuint)(val>>32));
	}
	void SetVec2(string name, glm::vec2 vec, ShaderTYPE type = ShaderTYPE::SIMPLE)
	{
		unsigned int usedID;
		if (type == ShaderTYPE::SIMPLE) usedID = ID;
		else usedID = computeID;
		int loc = glGetUniformLocation(usedID, name.c_str());
		if (loc == -1)
		{
#ifndef __HIDE_SHADER_LOGS__
			cout << "Didn't find uniform vec2 " << name << " in shader prog" << filename << endl;
#endif 
			return;
		}
		glUniform2fv(loc, 1, glm::value_ptr(vec));
	}
	void SetVec3(string name, glm::vec3 vec, ShaderTYPE type = ShaderTYPE::SIMPLE)
	{
		unsigned int usedID;
		if (type == ShaderTYPE::SIMPLE) usedID = ID;
		else usedID = computeID;
		int loc = glGetUniformLocation(usedID, name.c_str());
		if (loc == -1)
		{
#ifndef __HIDE_SHADER_LOGS__
			cout << "Didn't find uniform vec3 " << name << " in shader prog" << filename << endl;
#endif 
			return;
		}
		glUniform3fv(loc, 1, glm::value_ptr(vec));
	}
	void SetVec4(string name, glm::vec4 vec, ShaderTYPE type = ShaderTYPE::SIMPLE)
	{
		unsigned int usedID;
		if (type == ShaderTYPE::SIMPLE) usedID = ID;
		else usedID = computeID;
		int loc = glGetUniformLocation(usedID, name.c_str());
		if (loc == -1)
		{
#ifndef __HIDE_SHADER_LOGS__
			cout << "Didn't find uniform vec4 " << name << " in shader prog" << filename << endl;
#endif 
			return;
		}
		glUniform4fv(loc, 1, glm::value_ptr(vec));
	}
	void SetMat4(string name, glm::mat4 matrix, ShaderTYPE type = ShaderTYPE::SIMPLE)
	{
		unsigned int usedID;
		if (type == ShaderTYPE::SIMPLE) usedID = ID;
		else usedID = computeID;
		int loc = glGetUniformLocation(usedID, name.c_str());
		if (loc == -1)
		{
#ifndef __HIDE_SHADER_LOGS__
			cout << "Didn't find uniform mat4 " << name << " in shader prog" << filename << endl;
#endif 
			return;
		}
		glUniformMatrix4fv(loc, 1, false, glm::value_ptr(matrix));
	}
};

#endif
