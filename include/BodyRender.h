#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<stb_image.h>
#include<iostream>
#include<vector>
#include<cmath>
#include<thread>
#include<future>
#include<string>
#include "Shader.h"
#include "Interface.h"
#define PI 3.14
#define PROC_LAUNCHED 1
#define PROC_COMPLETED (1<<1)
//MAX AMOUNT OF OBJECTS 50
const float k = 0.1f; //gravitational constant
const float dt = 0.00005f; //delta time between handling frames
float time_scale = 0.4f; //modeling time and real time reference
float traj_interval = 0.04f; //interval between points marking trajectory
class Body;
std::vector<Body*> objects;
enum class Proc_Time
{
	PROG_LAUNCH, //when program only launches 
	PROG_EXEC  //while its full fledge execution
};
struct SSBO_data
{
	glm::mat4 transform;
	int mask;
	unsigned int if_draw_instanced; //to configure if we want to draw it with other instances or separately
	GLuint64 TexHandle;
	float vertices[20];
	
};
union BODY_FLAGS
{
private:
	char flags;
public:
	struct
	{
		unsigned char clicked : 1;
		unsigned char hovered : 1;
		unsigned char draw_trajectory : 1;
	};
	void clear()
	{
		clicked = 0;
		hovered = 0;
		draw_trajectory = 0;
	}
};

struct FUT_TYPE
{
	std::vector<vec2sq<float>> traj;
	std::vector<vec2sq<float>> vel;
	std::vector<vec2sq<float>> display_traj;
	FUT_TYPE(std::vector<vec2sq<float>> traj_vec, std::vector<vec2sq<float>> vel_vec, std::vector<vec2sq<float>> display_traj_vec)
		: traj(traj_vec), vel(vel_vec), display_traj(display_traj_vec) {}
};
class Body
{
private:
	std::vector<vec2sq<float>> trajectory, velocities, display_traj;
	vec2sq<float> initPos;
	vec2sq<float> initVel;
	std::future<FUT_TYPE> fut;
	unsigned char th_flags = 0;
	Proc_Time when;
	unsigned int texture, trajVAO, trajVBO;
	double startTime;
	vec2sq<float> position;
	float scaling_param;
	float trajIntensity = 0.0f; //trajectory color intensity
	int objectIndex;
	unsigned int selfVAO, selfVBO, selfEBO;
	Shader* selfShader;
	
	inline static void setUp(GLFWwindow* window)
	{
		if (FBO == 0)
		{
			win = window;
			winData = (windowData*)glfwGetWindowUserPointer(window);
			glGenTextures(1, &texColor);
			glBindTexture(GL_TEXTURE_2D, texColor);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, winData->WIDTH, winData->HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);

			glGenTextures(1, &texMask);
			glBindTexture(GL_TEXTURE_2D, texMask);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, winData->WIDTH, winData->HEIGHT, 0, GL_RED_INTEGER, GL_INT, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);

			glGenRenderbuffers(1, &renderBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, winData->WIDTH, winData->HEIGHT);

			glGenFramebuffers(1, &FBO);
			glBindFramebuffer(GL_FRAMEBUFFER, FBO);
			glBindTexture(GL_TEXTURE_2D, texColor);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0);
			glBindTexture(GL_TEXTURE_2D, texMask);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texMask, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				std::cout << "ERROR::FRAMEBUFFER_INCOMPLETE" << std::endl;
				glfwDestroyWindow(window);
				glfwTerminate();
				return;
			}
			
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
			glGenBuffers(1, &allObjSSBO);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, allObjSSBO);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, allObjSSBO);
			glBufferData(GL_SHADER_STORAGE_BUFFER, 50 * sizeof(SSBO_data), nullptr, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			
			glGenVertexArrays(1, &allObjVAO);
		}
	}

	void predictioning(std::vector<vec2sq<float>>& Traj, std::vector<vec2sq<float>>& Vel, std::vector<vec2sq<float>>& Display_traj)
	{
		int count = 0;
		vec2sq<float> prevPos = initPos;
		vec2sq<float> prevVel = initVel;
		float len = 0.0f;
		float angle = 0.0f;
		Display_traj.push_back(initPos);
		while (abs(angle) < 2 * PI)
		{
			Traj.push_back(prevPos);
			Vel.push_back(prevVel);
			float r = sqrt(prevPos.x * prevPos.x + prevPos.y * prevPos.y);
			count++;
			if (r > 1e-6)
			{
				vec2sq<float> prev_r = prevPos;
				vec2sq<float> acceleration = { -k * prevPos.x / (r * r * r),  -k * prevPos.y / (r * r * r) };
				prevPos += prevVel * dt + acceleration * (dt * dt / 2);
				prevVel += acceleration * dt;
				vec2sq<float> new_r = prevPos;
				angle += prev_r ^ new_r;
				len += sqrtf((new_r.x - prev_r.x) * (new_r.x - prev_r.x) + (new_r.y - prev_r.y) * (new_r.y - prev_r.y));
				if (len >= traj_interval)
				{
					Display_traj.push_back(new_r);
					len = 0.0f;
				}
			}
			else
			{
				std::cout << "Underfind behaviour" << std::endl;
			}

		}
	}
	void predict_trajectory()
	{
		if (when == Proc_Time::PROG_EXEC)
		{

			if (((th_flags & PROC_LAUNCHED) == 0))
			{
				fut = std::async(std::launch::async,
					[this]()
					{
						std::vector<vec2sq<float>> traj;
						std::vector<vec2sq<float>> vels;
						std::vector<vec2sq<float>> display_traj;
						traj.reserve(15000);
						vels.reserve(15000);
						predictioning(traj, vels, display_traj);
						return FUT_TYPE(traj, vels, display_traj);
					});
				th_flags |= PROC_LAUNCHED;
				std::cout << "start" << std::endl;
			}
			else if ((th_flags & PROC_LAUNCHED) != 0)
			{
				if (fut.valid() && fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
				{
					th_flags = 0;
					th_flags |= PROC_COMPLETED;
					th_flags &= ~PROC_LAUNCHED;
					auto results = fut.get();
					trajectory = results.traj;
					velocities = results.vel;
					display_traj = results.display_traj;
					glGenVertexArrays(1, &trajVAO);
					glGenBuffers(1, &trajVBO);
					glBindVertexArray(trajVAO);
					glBindBuffer(GL_ARRAY_BUFFER, trajVBO);
					glBufferData(GL_ARRAY_BUFFER, display_traj.size() * sizeof(vec2sq<float>), display_traj.data(), GL_STATIC_DRAW);
					glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(vec2sq<float>), 0);
					glEnableVertexAttribArray(0);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindVertexArray(0);
					std::cout << trajectory.size() << std::endl;
				}
			}
		}
		else if (when == Proc_Time::PROG_LAUNCH)
		{
			std::thread th([this]()
				{
					trajectory.reserve(15000);
					velocities.reserve(15000);
					predictioning(trajectory, velocities, display_traj);
				});
			th.join();
			th_flags |= PROC_COMPLETED;
			glGenVertexArrays(1, &trajVAO);
			glGenBuffers(1, &trajVBO);
			glBindVertexArray(trajVAO);
			glBindBuffer(GL_ARRAY_BUFFER, trajVBO);
			glBufferData(GL_ARRAY_BUFFER, display_traj.size() * sizeof(vec2sq<float>), display_traj.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(vec2sq<float>), 0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
	}
	void move()
	{
		if (trajectory.size() == 0)
		{
			position = { initPos.x, initPos.y * scaling_param };
		}
		else
		{
			float timeNow = glfwGetTime();
			int index = (int)(time_scale * (timeNow - startTime) / dt) % trajectory.size();
			position = { trajectory[index].x, trajectory[index].y * scaling_param };
		}
		objData.transform = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
	}
	void set_draw_traj_flag()
	{
		if (flags.clicked == 1)
			flags.draw_trajectory = 1;
		else if (flags.clicked == 0)
			flags.draw_trajectory = 0;
	}
	void trajChangeIntense()
	{
		if (flags.clicked == 1 && trajIntensity < 0.7f)
			trajIntensity += 0.01f;

		else if (flags.clicked == 0 && trajIntensity > 0.01f)
			trajIntensity -= 0.01f;

	}
	void drawTrajectory(Shader& shader)
	{
		trajChangeIntense();
		glBindVertexArray(trajVAO);
		shader.use();
		shader.SetFloat("scale_param", scaling_param);
		shader.SetFloat("intense", trajIntensity);
		glDrawArrays(GL_POINTS, 0, display_traj.size());
	}
	
	
	
public:
	BODY_FLAGS flags;
	SSBO_data objData;
	inline static unsigned int FBO = 0;
	inline static GLFWwindow* win = 0;
	inline static windowData* winData = 0;
	inline static unsigned int texColor = 0;
	inline static unsigned int texMask = 0;
	inline static unsigned int renderBuffer = 0;
	inline static unsigned int allObjSSBO = 0;
	inline static unsigned int allObjVAO = 0;

	Body(GLFWwindow* window, std::string filename, Shader* separateShader, const Proc_Time time,  const vec2sq<float>& initialPos, const vec2sq<float>& initialVel, float size, int Mask)
		: initPos(initialPos), initVel(initialVel), when(time), selfShader(separateShader)
	{
		flags.clear();
		setUp(window);
		scaling_param = winData->WIDTH * 1.0f/ winData->HEIGHT;
		float vertices[20] =
		{
			-size, -size*scaling_param, 0.0f,     0.0f, 0.0f,
			-size, size*scaling_param, 0.0f,      0.0f, 1.0f,
			size, size*scaling_param, 0.0f,       1.0f, 1.0f,
			size, -size*scaling_param, 0.0f,      1.0f, 0.0f
		};
		std::vector<unsigned int> vert_indeces =
		{
			0, 1, 2,    0, 2, 3
		};
		if ((initialPos.x == 0.0f && initialPos.y == 0.0f) && (initialVel.x == 0.0f && initialVel.y == 0.0f))
		{
			position = { 0.0f, 0.0f };
		}
		else
		{
			predict_trajectory();
			
		}
		int width, height;
		GLenum format;
		unsigned char* img_data;
		load_image(filename, img_data, width, height, format);
		generateTexture(texture, img_data, width, height, format);
		stbi_image_free(img_data);
		objects.push_back(this);
		objectIndex = objects.size() - 1;
		std::memcpy(objData.vertices, vertices, 20 * sizeof(float));
		objData.mask = Mask;
		objData.TexHandle = glGetTextureHandleARB(texture);
		objData.if_draw_instanced = 1;
		glMakeTextureHandleResidentARB(objData.TexHandle);
		
		glGenVertexArrays(1, &selfVAO);
		glGenBuffers(1, &selfVBO);
		glGenBuffers(1, &selfEBO);
		glBindVertexArray(selfVAO);
		glBindBuffer(GL_ARRAY_BUFFER, selfVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, selfEBO);
		glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), vertices, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), vert_indeces.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		startTime = glfwGetTime();
	}
	void update()
	{
		check_if_traj_is_built();
		check_if_clicked();
		move();
		trajChangeIntense();
		set_draw_traj_flag();
		if (flags.clicked == 1 || flags.hovered == 1 || trajIntensity>0.01f)
			objData.if_draw_instanced = 0;
			
		else objData.if_draw_instanced = 1;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, allObjSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, objectIndex * sizeof(SSBO_data), sizeof(SSBO_data), &objData);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		
	}
	void drawBodySeparately(Shader& trajShader)
	{
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		glBindVertexArray(selfVAO);
		selfShader->use();
		selfShader->SetMat4("transform", objData.transform);
		selfShader->SetInt("type", 0);
		selfShader->SetInt("mask", objData.mask);
		selfShader->Set2UInt("TexHandle", objData.TexHandle);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		if (flags.hovered == 1)
		{
			selfShader->use();
			selfShader->SetMat4("transform", objData.transform);
			selfShader->SetInt("type", 1);
			selfShader->SetInt("mask", objData.mask);
			selfShader->Set2UInt("TexHandle", objData.TexHandle);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
		if (flags.draw_trajectory == 1)
		{
			drawTrajectory(trajShader);

		}
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
	}
	void check_if_traj_is_built()
	{
		if (when == Proc_Time::PROG_EXEC)
		{
			if ((th_flags & PROC_COMPLETED) == 0)
				predict_trajectory();
		}
	}
	void check_if_clicked()
	{
		if (flags.hovered == 1 && winData->mouse_pressed == true)
			flags.clicked = 1;
		else if (flags.hovered == 0 && winData->mouse_pressed == true)
			flags.clicked = 0;
	}
	
	
};
int MouseHoverMask();
void drawingInFBO(Shader& bodyShader, Shader& trajShader)
{
	glBindFramebuffer(GL_FRAMEBUFFER, Body::FBO);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_STENCIL_TEST);

	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
	int clearMask = 0;
	glClearBufferiv(GL_COLOR, 1, &clearMask);
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, clearColor);
	GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, attachments);
	for (int i = 0; i < objects.size(); i++)
	{
		objects[i]->update();
	}
	glBindVertexArray(Body::allObjVAO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, Body::allObjSSBO);
	bodyShader.use();
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, objects.size());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glBindVertexArray(0);
	for (int i = 0; i < objects.size(); i++)
	{
		if (objects[i]->objData.if_draw_instanced == 0)
			objects[i]->drawBodySeparately(trajShader);
	}
	int mask = MouseHoverMask();
	for (int i = 0; i < objects.size(); i++)
	{
		if (objects[i]->objData.mask == mask)
		{
			objects[i]->flags.hovered = 1;
			
			
		}
		else if (objects[i]->flags.hovered != 0)
		{
			objects[i]->flags.hovered = 0;
			
		}
			
	}
	glDisable(GL_PROGRAM_POINT_SIZE);
	glDisable(GL_STENCIL_TEST);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
}
int MouseHoverMask()
{
	double mouseX, mouseY;
	glfwGetCursorPos(Body::win, &mouseX, &mouseY);
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	int mask;
	glReadPixels(mouseX, Body::winData->HEIGHT-mouseY, 1, 1, GL_RED_INTEGER, GL_INT, &mask);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	return mask;
}
void drawOnScreen(unsigned int& scrVAO, unsigned int& texColor, Shader& scrShader)
{
	glBindVertexArray(scrVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texColor);
	scrShader.use();
	scrShader.SetInt("Tex", 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
