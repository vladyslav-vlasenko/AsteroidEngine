
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<iostream>
#include "Interface.h"
#include "BodyRender.h"
#include "Functions.h"
#define S SHADERS_PATH
#define A ASSETS_PATH
int WIDTH = 650;
int HEIGHT = 900;
float scr_vert[] =
{
	-1.0f, -1.0f, 0.0f,    0.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,     0.0f, 1.0f,
	1.0f, 1.0f, 0.0f,      1.0f, 1.0f,
	1.0f, -1.0f, 0.0f,     1.0f, 0.0f,
};
unsigned int scr_vert_ind[] =
{
	0, 1, 2,    0, 2, 3
};
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Asteroid", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "ERROR::CAN'T_GET_FUNCTIONS_ADDRESS" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}
	 //all objects in the system

	unsigned char* cursor_data;
	int cur_width, cur_height;
	load_cursor_image(A "Interface/cursor.png", cursor_data, cur_width, cur_height);
	GLFWimage cursor_img;
	cursor_img.pixels = cursor_data;
	cursor_img.width = cur_width;
	cursor_img.height = cur_height;
	GLFWcursor* cursor = glfwCreateCursor(&cursor_img, cur_width/2, cur_height/2);
	stbi_image_free(cursor_data);
	glfwSetCursor(window, cursor);
	glfwSetMouseButtonCallback(window, mousebuttonclick_callback);
	glfwSetCursorPosCallback(window, mousemove_callback);
	glfwShowWindow(window);
	windowData winData;
	winData.WIDTH = WIDTH;
	winData.HEIGHT = HEIGHT;
	winData.mouse_pressed = false;
	glfwSetWindowUserPointer(window, &winData);
	Shader global_body_shader(S "global_body_vertex_shader.vs", S "global_body_fragment_shader.fs");
	Shader indiv_body_shader(S "indiv_body_vertex_shader.vs", S "indiv_body_fragment_shader.fs");
	Shader trajectory_shader(S "trajectory_vertex_shader.vs", S "trajectory_fragment_shader.fs");
	Shader screen_shader(S "screen_vertex_shader.vs", S "screen_fragment_shader.fs");
	Shader button_shader(S "button_vertex_shader.vs", S "button_fragment_shader.fs");
	Shader interface_shader(S "screen_vertex_shader.vs", S "interface_fragment_shader.fs");
	Shader buttonblock_shader(S "buttonblock_vertex_shader.vs", S "buttonblock_fragment_shader.fs");
	Body sun(window, A "Bodies/sun.jpg", &indiv_body_shader,Proc_Time::PROG_LAUNCH, { 0.0f, 0.0f }, { 0.0f, 0.0f }, 0.05f, 10);
	Body earth(window, A "Bodies/earth.png", &indiv_body_shader, Proc_Time::PROG_LAUNCH, { 0.3f, 0.0f }, { 0.0f, 0.67f }, 0.04f, 20);
	standardCallBacksForBlocks::VerticalDisplay_args vec;
	std::vector <std::string> filenames = { A "Interface/add_object_button.png", A "Interface/delete_object_button.png", A "Interface/show_all_trajectories.png"};
	std::vector <int> Masks = { 30, 40, 50 };
	std::vector <void (*)(Button*, void*)> BlockFunctions = {add_object_func, NULL, show_all_traj_func};
	add_object_struct add_obj_args(window, &indiv_body_shader, nullptr);
	std::vector <void*> BlockFunctionsArgs = { (void*)&add_obj_args, NULL, NULL };
	ButtonBlock block1(window, filenames, buttonblock_shader, Masks, STANDARD_RIGHT_MOUSEBUTTON_CALL, STANDARD_LEFT_MOUSEBUTTON_HIDE, standardCallBacksForBlocks::VerticalDisplay, BlockFunctions, BlockFunctionsArgs, NULL, NULL, (void*)(&vec));
	add_obj_args.block = &block1;
	Slider slider(window, A "Interface/slider_frame_img.png", A "Interface/slider_point_img.png", vec2sq<float>(0.8f, 0.8f), button_shader, button_shader, 80, true);

	unsigned int scrVAO, scrVBO, scrEBO;
	glGenVertexArrays(1, &scrVAO);
	glGenBuffers(1, &scrVBO);
	glGenBuffers(1, &scrEBO);
	glBindVertexArray(scrVAO);
	glBindBuffer(GL_ARRAY_BUFFER, scrVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scrEBO);
	glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), scr_vert, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), scr_vert_ind, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	
	
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		drawingInFBO(global_body_shader, trajectory_shader);
		glBindVertexArray(scrVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Body::texColor);
		screen_shader.use();
		screen_shader.SetInt("Tex", 0);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		block1.blockCall();
		Button::displayButtons(interface_shader, scrVAO);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyCursor(cursor);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}	