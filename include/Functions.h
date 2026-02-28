#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<iostream>
#include "Interface.h"
#include "BodyRender.h"
#define S SHADERS_PATH
#define A ASSETS_PATH
struct add_object_struct
{
	GLFWwindow* window;
	Shader* indivShader;
	ButtonBlock* block;
	add_object_struct(GLFWwindow* win, Shader* indiv_shader, ButtonBlock* but_block) : window(win), block(but_block), indivShader(indiv_shader) {}
};
void add_object_func(Button*, void* args)
{
	add_object_struct* casted_args = (add_object_struct*)args;
	windowData* winData = (windowData*)glfwGetWindowUserPointer(casted_args->window);
	Shader* indiv_shader = casted_args->indivShader;
	vec2sq<float> callPos_modified = vec2sq<float>(casted_args->block->targetPos.x, (winData->HEIGHT * 1.0f / winData->WIDTH) * casted_args->block->targetPos.y);
	Body* new_obj = new Body(casted_args->window, A "Bodies/earth.png", indiv_shader, Proc_Time::PROG_EXEC, callPos_modified, { 0.0f, 0.4f }, 0.04f, (objects.size() + 1) * 10);
}
void show_all_traj_func(Button*, void* args)
{
	for (int i = 0; i < objects.size(); i++)
	{
		objects[i]->flags.draw_trajectory = 1;
		objects[i]->flags.clicked = 1;
	}
}