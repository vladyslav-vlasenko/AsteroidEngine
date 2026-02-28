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

#define BUTTONBLOCK_CALLED 1
#define BUTTONBLOCK_CALLED_FIRST (1<<1)
#define BUTTONBLOCK_WAIT_TO_CLOSE (1<<2)

#define STANDARD_RIGHT_MOUSEBUTTON_CALL standardCallBacksForBlocks::RightMouseButtonCall
#define STANDARD_LEFT_MOUSEBUTTON_HIDE standardCallBacksForBlocks::OnePressHide
#define STANDARD_VERTICAL_DISPLAY standardCallBacksForBlocks::VerticalDisplay


union BUTTON_FLAGS
{
private:
	short int flags;
public:
	struct
	{
		unsigned char if_called : 1;
		unsigned char if_pressed : 1; //if 1 - button was in waits_to_be_pressed state but left mouse button was released (button is hovered)
//- so buttons' funcs will start their execution
		unsigned char if_waits_to_be_pressed : 1; //if 1 - button is hovered and left mouse button is pressed (but still not released)
		unsigned char if_hovered : 1;
		unsigned char if_static : 1;
		unsigned char if_display : 1;
		unsigned char if_exec : 1;
		unsigned char if_in_block : 1;
		unsigned char if_in_slider : 1;
		
	};
	void clear()
	{
		if_called = 0;
		if_pressed = 0;
		if_waits_to_be_pressed = 0;
		if_hovered = 0;
		if_static = 0;
		if_display = 0;
		if_exec = 0;
		if_in_block = 0;
		if_in_slider = 0;
	}
};


template <typename T>
struct vec2sq
{
	T x, y;
	vec2sq(T X, T Y) : x(X), y(Y) {}
	vec2sq() {}
	friend vec2sq<T> operator+(const vec2sq<T>& a, const vec2sq<T>& b)
	{
		return { a.x + b.x, a.y + b.y };
	}
	friend vec2sq<T> operator*(const T& num, const vec2sq<T>& vec)
	{
		return { num * vec.x, num * vec.y };
	}
	friend vec2sq<T> operator*(const vec2sq<T>& vec, const T& num)
	{
		return { num * vec.x, num * vec.y };
	}
	vec2sq<T>& operator+=(const vec2sq<T>& delta)
	{
		this->x += delta.x;
		this->y += delta.y;
		return *this;
	}
	//determination of angle using vector product
	friend float operator^(const vec2sq<T>& vec1, const vec2sq<T>& vec2)
	{
		float vec_prod_z = vec1.x * vec2.y - vec1.y * vec2.x;
		float scalar_prod = vec1.x * vec2.x + vec1.y * vec2.y;
		float abs_vec1 = sqrt(vec1.x * vec1.x + vec1.y * vec1.y);
		float abs_vec2 = sqrt(vec2.x * vec2.x + vec2.y * vec2.y);
		float sin = vec_prod_z / (abs_vec1 * abs_vec2);
		float cos = scalar_prod / (abs_vec1 * abs_vec2);
		return atan2f(sin, cos);
	}
	friend bool operator==(const vec2sq<T>& vec1, const vec2sq<T>& vec2)
	{
		if (vec1.x == vec2.x && vec1.y == vec2.y)
			return true;
		else return false;
	}
	friend bool operator!=(const vec2sq<T>& vec1, const vec2sq<T>& vec2)
	{
		if (vec1.x != vec2.x || vec1.y != vec2.y)
			return true;
		else return false;
	}
};
struct windowData
{
	int WIDTH, HEIGHT;
	bool mouse_pressed;
	vec2sq<double> mouse_pos_change;
};
void mousemove_callback(GLFWwindow* window, double xpos, double ypos)
{
	static double prevX = xpos;
	static double prevY = ypos;
	double xoffset = xpos - prevX;
	double yoffset = prevY - ypos;
	windowData* winData = (windowData*)glfwGetWindowUserPointer(window);
	winData->mouse_pos_change = vec2sq<double>(xoffset, yoffset);
}
void mousebuttonclick_callback(GLFWwindow* window, int button, int action, int mods)
{
	windowData* winData = (windowData*)glfwGetWindowUserPointer(window);
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		winData->mouse_pressed = true;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		winData->mouse_pressed = false;
}
void load_cursor_image(std::string filename, unsigned char*& data, int& width, int& height)
{
	int nChannels;
	data = stbi_load(filename.c_str(), &width, &height, &nChannels, 4);
	for (int i = 0; i < 4 * width * height; i += 4)
	{
		if ((data[i] < 50 && data[i + 1] < 50) && data[i + 2] < 50)
		{
			data[i + 3] = 0;
		}
		else
		{
			data[i + 3] = 255;
		}
	}
}


void load_image(std::string filename, unsigned char*& data, int& width, int& height, GLenum& format)
{
	int nChannel;
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load(filename.c_str(), &width, &height, &nChannel, 0);
	if (!data)
	{
		std::cout << "ERROR::WHILE_IMAGE_LOADING" << std::endl;
		return;
	}
	switch (nChannel)
	{
	case 4:
	{
		format = GL_RGBA;
		break;
	}
	case 3:
	{
		format = GL_RGB;
		break;
	}
	}
};
void generateTexture(unsigned int& texture, unsigned char* data, int& width, int& height, GLenum format)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}


class Button;
class ButtonBlock;
class Slider;
struct allButtons_type
{
	Button* button;
	ButtonBlock* block;
	Slider* slider;
	allButtons_type(Button* but, ButtonBlock* bl, Slider* sl) : button(but), block(bl), slider(sl) {}
};
std::vector<allButtons_type> allButtons;

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

class Button
{
private:
	
	unsigned int VAO, VBO, EBO, texture;
	glm::mat4 transform;
	float border_size = 0.05f;
	glm::vec2 border_scale_vec;
	inline static unsigned int FBO = 0;
	inline static unsigned int texMask = 0; //texture that stores masked pixels for FBO
	inline static unsigned int texColor = 0; //texture that stores colorful pixels for FBO
	inline static unsigned int renderBuffer = 0; //render buffer that stores stencil value for FBO

	
	inline static void  createFBO(GLFWwindow* window)
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

			glGenTextures(1, &texMask);
			glBindTexture(GL_TEXTURE_2D, texMask);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, winData->WIDTH, winData->HEIGHT, 0, GL_RED_INTEGER, GL_INT, nullptr);

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
			glBindRenderbuffer(GL_FRAMEBUFFER, 0);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				std::cout << "ERROR::FRAMEBUFFER_INCOMPLETE_IN_BUTTON" << std::endl;
				return;
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

public:
	vec2sq<float> size;
	vec2sq<float> pos;
	void (*button_callback)(Button*, void*);
	void* button_callback_args;
	Shader* shader;
    int mask;
	BUTTON_FLAGS flags;
	inline static GLFWwindow* win = nullptr;
	inline static windowData* winData = nullptr;
    Button(GLFWwindow* window, Shader& Shader, std::string filename, float sizeX, float sizeY, int Mask, bool if_static, void (*buttonCallback)(Button* button, void* args), 
		void* args, ButtonBlock* block = NULL, Slider* slider = NULL)
		: shader(&Shader), mask(Mask), button_callback(buttonCallback), button_callback_args(args)
	{
		
		flags.clear();
		createFBO(window);
		unsigned char* img_data;
		int width, height;
		GLenum format;
		load_image(filename, img_data, width, height, format);
		generateTexture(texture, img_data, width, height, format);
		stbi_image_free(img_data);
		if (if_static)
		{
			flags.if_static = true;
			flags.if_display = true; 
		}
		float dimX, dimY;
		if (sizeX == 0 && sizeY == 0)
		{
			dimX = 2 * width * 1.0 / winData->WIDTH;
			dimY = 2 * height * 1.0 / winData->HEIGHT;
		}
		else
		{
			dimX = sizeX;
			dimY = sizeY;
		}
		size = vec2sq<float>(dimX, dimY);
		float vert_coord[20] =
		{
			-dimX / 2, -dimY / 2, 0.0f,     0.0f, 0.0f,
			-dimX / 2, dimY / 2, 0.0f,      0.0f, 1.0f,
			dimX / 2, dimY / 2, 0.0f,       1.0f, 1.0f,
			dimX / 2, -dimY / 2, 0.0f,      1.0f, 0.0f
		};
		unsigned int vert_ind[6] =
		{
			0, 1, 2,    0, 2, 3
		};
		border_scale_vec = glm::vec2(1.0f + border_size/size.x,  1.0f + border_size/size.y);
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), vert_coord, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), vert_ind, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		allButtons.push_back(allButtons_type(this, block, slider));
	}
	virtual ~Button()
	{
		for (int i = 0; i < allButtons.size(); i++)
		{
			if (allButtons[i].button == this)
			{
				allButtons.erase(allButtons.begin() + i);
				break;
			}
		}
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		glDeleteTextures(1, &texture);
		if (allButtons.size() == 0)
		{
			glDeleteTextures(1, &texColor);
			glDeleteTextures(1, &texMask);
			glDeleteRenderbuffers(1, &renderBuffer);
			glDeleteFramebuffers(1, &FBO);
		}
	}
	virtual void drawInFBO()
	{
		transform = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		
		if (flags.if_hovered == 1)
		{
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilMask(0xFF);
		}
		shader->use();
		shader->SetInt("type", 0);
		shader->SetMat4("transform", transform);
		shader->SetVec2("scale_vec", border_scale_vec);
		shader->SetInt("Tex", 0);
		shader->SetInt("Mask", mask);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		glStencilMask(0x00);
	}
	void drawButtonInBlocksInFBO(vec2sq<float> callPos)
	{
		transform = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		if (flags.if_hovered == 1)
		{
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilMask(0xFF);
		}
		shader->use();
		shader->SetInt("Type", 0);
		shader->SetInt("callPosY", (int)(winData->HEIGHT/2+callPos.y* winData->HEIGHT / 2));
		shader->SetMat4("transform", transform);
		shader->SetInt("Tex", 0);
		shader->SetInt("Mask", mask);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		glStencilMask(0x00);
	}
	void drawButtonBorder()
	{
		
		if (flags.if_hovered == 1)
		{
			glBindVertexArray(VAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			shader->use();
			shader->SetInt("type", 1);
			shader->SetMat4("transform", transform);
			shader->SetInt("Tex", 0);
			shader->SetInt("Mask", 0);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindVertexArray(0);
		}
		
	}
	void drawButtonInBlockBorder(vec2sq<float> callPos)
	{
		
		if (flags.if_hovered == 1)
		{
			glBindVertexArray(VAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			shader->use();
			shader->SetInt("Type", 1);
			shader->SetInt("callPosY", (int)(winData->HEIGHT / 2 + callPos.y * winData->HEIGHT / 2));
			shader->SetMat4("transform", transform);
			shader->SetInt("Tex", 0);
			shader->SetInt("Mask", 1);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindVertexArray(0);
		}
		
	}
	void checkIfButtonClicked()
	{
		if ((flags.if_hovered == 1 && winData->mouse_pressed == 1) && flags.if_waits_to_be_pressed == 0)
		{
			flags.if_waits_to_be_pressed = 1;
		}	
			
		else if ((winData->mouse_pressed == 0 && flags.if_hovered == 1) && flags.if_waits_to_be_pressed == 1)
		{
			flags.if_waits_to_be_pressed = 0;
			flags.if_pressed = 1;
			std::cout << "PRESSED" << std::endl;
		}
		else if (flags.if_waits_to_be_pressed == 0 && flags.if_pressed != 0)
		{
			flags.if_pressed = 0;
		}
		else if ((winData->mouse_pressed == 0 && flags.if_hovered == 0) && flags.if_waits_to_be_pressed == 1)
		{
			flags.if_waits_to_be_pressed = 0;
		}
	}
	void CallButton(bool if_call)
	{
		if (flags.if_static == 0)
		{
			if (if_call)
			{
				flags.if_called = 1;
				
			}
			else if (!if_call) flags.if_called = 0;
		}
	}

	void execCommand()
	{
		if (button_callback != NULL)
		{
			if (flags.if_pressed == 1 && flags.if_exec == 0)
			{
				button_callback(this, button_callback_args);
				flags.if_exec = true;
			}
			else if (flags.if_pressed == 0)
				flags.if_exec = 0;
		}
		
	}
	inline static void drawAllElementsInFBO();
	inline static void displayButtons(Shader& interfaceShader, unsigned int scrVAO)
	{
		Button::drawAllElementsInFBO();
		glBindVertexArray(scrVAO);
		interfaceShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texColor);
		interfaceShader.SetInt("texColor", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texMask);
		interfaceShader.SetInt("texMask", 1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}
	inline static int GetMouseMask()
	{
		if (winData->mouse_pos_change.x != 0 && winData->mouse_pos_change.y != 0)
		{
			double mouseX, mouseY;
			glfwGetCursorPos(win, &mouseX, &mouseY);
			glReadBuffer(GL_COLOR_ATTACHMENT1);
			int mask;
			glReadPixels(mouseX, winData->HEIGHT - mouseY, 1, 1, GL_RED_INTEGER, GL_INT, &mask);
			return mask;
		}
		return -1;
	}
};

/***********************************************************************/
/***********************************************************************/
/***********************************************************************/

class ButtonBlock
{

public:
	void (*call_func)(ButtonBlock*, void* args);
	void (*hide_func)(ButtonBlock*, void* args);
	void (*display_func)(ButtonBlock*, void*);
	void* call_func_args;
	void* hide_func_args;
	void* display_func_args;
	unsigned char flags;
	std::vector<Button*> buttons;
	vec2sq<float> callPos;
	vec2sq<float> targetPos;
	vec2sq<float> blockSize;
	int active_button_num;
	ButtonBlock(GLFWwindow* window, std::vector<std::string>& filenames, Shader& shader, std::vector<int>& Masks, 
		void (*CallFunctionCallback)(ButtonBlock*, void* args), void (*HideFunctionCallback)(ButtonBlock* block, void* args), 
		void (*DisplayingFunctionCallback)(ButtonBlock*, void* args), std::vector<void (*)(Button*, void*)> ButtonsFunction, std::vector<void*>& buttons_function_args,
		void* call_args = NULL, void* hide_args = NULL, void* display_args = NULL)
		: call_func(CallFunctionCallback), call_func_args(call_args), hide_func(HideFunctionCallback), hide_func_args(hide_args), 
		display_func(DisplayingFunctionCallback), display_func_args(display_args)
	{	
		blockSize.x = 0;
		blockSize.y = 0;
		active_button_num = -1;
		callPos = vec2sq<float>(0.0f, 0.0f);
		targetPos = vec2sq<float>(0.0f, 0.0f);
		flags = 0;
		for (int i = 0; i < filenames.size(); i++)
		{
			Button* but = new Button(window, shader, filenames[i], 0, 0, Masks[i], false, ButtonsFunction[i], buttons_function_args[i], this);
			buttons.push_back(but);
			but->flags.if_in_block = true;
		}
	}
	~ButtonBlock()
	{
		
		for (int i = 0; i < buttons.size(); i++)
		{
			delete buttons[i];
		}
	}
	void blockCall()
	{
		call_func(this, call_func_args);
		hide_func(this, hide_func_args);
		display_func(this, display_func_args);
	}
	
};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

class Slider : public Button
{
private:
	unsigned int frameVAO, frameVBO, frameEBO, frameTex;
	Shader* frameShader;
	vec2sq<float> callPos;
	vec2sq<float> framePosition;
	vec2sq<float> frameSize;
	glm::mat4 frameTransform;
	float frame_border_size = 0.03f;
	glm::vec2 frame_border_scale_vec;
	float slider_value; //given in interval [0; 1]
	void slider_func()
	{
		if (flags.if_waits_to_be_pressed == 1)
		{
			double mouseX, mouseY;
			glfwGetCursorPos(Button::win, &mouseX, &mouseY);
			float mouseX_norm = (2 * mouseX - Button::winData->WIDTH) * 1.0f / Button::winData->WIDTH;
			check_slider_pos(mouseX_norm);
			count_result();
		}
		
	}
	void check_slider_pos(float mouseX_norm)
	{
		if (mouseX_norm > (framePosition + 0.5 * frameSize).x)
			pos.x = (framePosition + 0.5 * frameSize).x;
		else if (mouseX_norm < (framePosition + (-0.5) * frameSize).x)
			pos.x = (framePosition + (-0.5) * frameSize).x;
		else
			pos.x = mouseX_norm;
	}
	void count_result()
	{
		slider_value = (pos.x - (framePosition.x - frameSize.x / 2))/frameSize.x;
	}
public:
	Slider(GLFWwindow* window, std::string sliderFrame_img, std::string sliderPoint_img, vec2sq<float> sliderPos, Shader& button_shader, Shader& frame_shader,
		int Mask, bool if_static) : Button(window, button_shader, sliderPoint_img, 0.0f, 0.0f, Mask, if_static, NULL, NULL, NULL, this), 
		frameShader(&frame_shader), framePosition(sliderPos)
	{
		flags.if_in_slider = 1;
		flags.if_display = 1;
		
		unsigned char* frameImgData;
		int frameImgWidth, frameImgHeight;
		GLenum frameImgFormat;
		load_image(sliderFrame_img, frameImgData, frameImgWidth, frameImgHeight, frameImgFormat);
		generateTexture(frameTex, frameImgData, frameImgWidth, frameImgHeight, frameImgFormat);
		stbi_image_free(frameImgData);
		float sizeX_norm = 2 * frameImgWidth * 1.0f/ Button::winData->WIDTH;
		float sizeY_norm = 2 * frameImgHeight * 1.0f/ Button::winData->HEIGHT;
		frameSize.x = sizeX_norm;
		frameSize.y = sizeY_norm;
		frame_border_scale_vec = glm::vec2(1.0f + frame_border_size/frameSize.x, 1.0f + frame_border_size / frameSize.y);
		pos = framePosition;
		float vertices[] =
		{
			-sizeX_norm / 2, -sizeY_norm / 2, 0.0f,      0.0f, 0.0f,
			-sizeX_norm / 2, sizeY_norm / 2, 0.0f,       0.0f, 1.0f,
			sizeX_norm / 2, sizeY_norm / 2, 0.0f,        1.0f, 1.0f,
			sizeX_norm / 2, -sizeY_norm / 2, 0.0f,       1.0f, 0.0f
		};
		unsigned int vert_indices[] =
		{ 0, 1, 2,     0, 2, 3 };
		glGenVertexArrays(1, &frameVAO);
		glGenBuffers(1, &frameVBO);
		glGenBuffers(1, &frameEBO);

		glBindVertexArray(frameVAO);
		glBindBuffer(GL_ARRAY_BUFFER, frameVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frameEBO);

		glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), vert_indices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		frameTransform = glm::translate(glm::mat4(1.0f), glm::vec3(framePosition.x, framePosition.y, 0.0f));
		std::cout << frameImgWidth << std::endl;
	}

	void drawInFBO() override
	{
		slider_func();
		glStencilFunc(GL_ALWAYS, 2, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		glBindVertexArray(frameVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, frameTex);
		frameShader->use();
		frameShader->SetMat4("transform", frameTransform);
		frameShader->SetInt("type", 0);
		frameShader->SetInt("Tex", 0);
		frameShader->SetInt("Mask", 18);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glStencilFunc(GL_NOTEQUAL, 2, 0xFF);
		glStencilMask(0x00);
		frameShader->use();
		frameShader->SetMat4("transform", frameTransform);
		frameShader->SetInt("type", 1);
		frameShader->SetVec2("scale_vec", frame_border_scale_vec);
		frameShader->SetInt("Tex", 0);
		frameShader->SetInt("Mask", 18);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		Button::drawInFBO();
	}
	
};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

namespace standardCallBacksForBlocks
{
	void RightMouseButtonCall(ButtonBlock* block, void*)
	{
		int right_mouse_button_state = glfwGetMouseButton(Button::win, GLFW_MOUSE_BUTTON_RIGHT);
		if (block->blockSize == vec2sq<float>(0, 0))
		{
			for (int i = 0; i < block->buttons.size(); i++)
			{
				block->blockSize.x = block->buttons[i]->size.x;
				block->blockSize.y += block->buttons[i]->size.y;
			}
		}
		if (right_mouse_button_state == GLFW_PRESS && (block->flags & BUTTONBLOCK_CALLED) == 0)
		{
			block->flags |= BUTTONBLOCK_CALLED;
			block->flags |= BUTTONBLOCK_CALLED_FIRST;
			double posX, posY;
			glfwGetCursorPos(Button::win, &posX, &posY);
			block->callPos.x = (2 * posX - Button::winData->WIDTH)*1.0f / Button::winData->WIDTH;
			block->callPos.y = (-2 * posY + Button::winData->HEIGHT)*1.0f / Button::winData->HEIGHT;
			block->targetPos = block->callPos;
			if (block->callPos.x < -0.99f) block->callPos.x = -0.99f;
			else if (block->callPos.x + block->blockSize.x > 0.99f) block->callPos.x = 0.99f - block->blockSize.x;
			if (block->callPos.y > 0.99f) block->callPos.y = 0.99f;
			else if (block->callPos.y - block->blockSize.y - 0.01f * block->buttons.size() < -0.99f) block->callPos.y = -0.99f + 0.01f * block->buttons.size() + block->blockSize.y;
			for (int i = 0; i < block->buttons.size(); i++)
			{
				block->buttons[i]->CallButton(true);
			}
		}
		else if (right_mouse_button_state == GLFW_PRESS && (block->flags & BUTTONBLOCK_CALLED) != 0)
		{
			block->flags &= ~BUTTONBLOCK_CALLED_FIRST;
		}
	}
	void OnePressHide(ButtonBlock* block, void*)
	{
		int left_mouse_button_state = glfwGetMouseButton(Button::win, GLFW_MOUSE_BUTTON_LEFT);
		if ((left_mouse_button_state == GLFW_PRESS && (block->flags & BUTTONBLOCK_WAIT_TO_CLOSE) == 0) && (block->flags & BUTTONBLOCK_CALLED) != 0)
		{
			for (int i = 0; i < block->buttons.size(); i++)
			{
				if (block->buttons[i]->flags.if_hovered == 1)
				{
					block->flags |= BUTTONBLOCK_WAIT_TO_CLOSE;
					block->active_button_num = i;
					break;
				}
			}
			if (block->active_button_num == -1)
			{
				for (int i = 0; i < block->buttons.size(); i++)
				{
					block->buttons[i]->CallButton(false);
				}
				block->flags &= ~BUTTONBLOCK_CALLED;
			}
		}
		else if (left_mouse_button_state == GLFW_RELEASE && (block->flags & BUTTONBLOCK_WAIT_TO_CLOSE) != 0)
		{
			for (int i = 0; i < block->buttons.size(); i++)
			{
				block->buttons[i]->CallButton(false);
			}
			block->flags &= ~BUTTONBLOCK_WAIT_TO_CLOSE;
			block->flags &= ~BUTTONBLOCK_CALLED;
			block->active_button_num = -1;
		}
			
	}
	struct VerticalDisplay_args
	{
		std::vector<vec2sq<float>> targetButtonPoses;
		VerticalDisplay_args() {}
	};
	void VerticalDisplay(ButtonBlock* block, void* args) //args = (std::vector<vec2sq<float>> targetButtonPoses)
	{
		VerticalDisplay_args* casted_args = (VerticalDisplay_args*)args;
		if (casted_args->targetButtonPoses.size() == 0)
		{
			casted_args->targetButtonPoses.push_back(vec2sq<float>(block->buttons[0]->size.x / 2, -block->buttons[0]->size.y / 2));
			for (int i = 1; i < block->buttons.size(); i++)
			{
				casted_args->targetButtonPoses.push_back(casted_args->targetButtonPoses[i - 1] + vec2sq<float>(0.0f, -(block->buttons[i - 1]->size.y + block->buttons[i]->size.y + 0.01f) / 2));
			}
		}
		if ((block->flags & BUTTONBLOCK_CALLED_FIRST) != 0)
		{
			for (int i = 0; i < block->buttons.size(); i++)
			{
				block->buttons[i]->flags.if_display = true;
				block->buttons[i]->pos = block->callPos + vec2sq<float>(block->buttons[i]->size.x / 2, block->buttons[i]->size.y/2);
				
			}
		}
		if ((block->flags & BUTTONBLOCK_CALLED) != 0)
		{

			for (int i = 0; i < block->buttons.size(); i++)
			{
				if (block->buttons[i]->pos.y > block->callPos.y + casted_args->targetButtonPoses[i].y) block->buttons[i]->pos.y -= 0.002f;
			}
		}
		else if ((block->flags & BUTTONBLOCK_CALLED) == 0)
		{
			for (int i = 0; i < block->buttons.size(); i++)
			{
				if (block->buttons[i]->pos.y > block->callPos.y + block->buttons[i]->size.y / 2) block->buttons[i]->flags.if_display = false;
				else if (block->buttons[i]->pos.y <= block->callPos.y + block->buttons[i]->size.y / 2) block->buttons[i]->pos.y += 0.002f;
			}
		}
	}
	
}


inline void Button::drawAllElementsInFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glEnable(GL_STENCIL_TEST);
	glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glStencilMask(0x00);
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	float colorClear[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, colorClear);
	int maskClear = 0;
	glClearBufferiv(GL_COLOR, 1, &maskClear);
	for (int i = 0; i < allButtons.size(); i++)
	{
		allButtons[i].button->checkIfButtonClicked();
		allButtons[i].button->execCommand();
		
		if (allButtons[i].button->flags.if_display == 1 && allButtons[i].button->flags.if_in_block == 1)
			allButtons[i].button->drawButtonInBlocksInFBO(allButtons[i].block->callPos);
		else if (allButtons[i].button->flags.if_display == 1 && allButtons[i].button->flags.if_in_slider == 1)
			allButtons[i].slider->drawInFBO();
		else if (allButtons[i].button->flags.if_display == 1)
			allButtons[i].button->drawInFBO();
	}
	for (int i = 0; i < allButtons.size(); i++)
	{
		if ((allButtons[i].button->flags.if_display == 1 && allButtons[i].button->flags.if_called == 1)&& allButtons[i].button->flags.if_in_block == 0)
			allButtons[i].button->drawButtonBorder();
		else if ((allButtons[i].button->flags.if_display == 1 && allButtons[i].button->flags.if_called == 1) && allButtons[i].button->flags.if_in_block == 1)
			allButtons[i].button->drawButtonInBlockBorder(allButtons[i].block->callPos);
	}
	int mask = GetMouseMask();
	if (mask != -1)
	{
		for (int i = 0; i < allButtons.size(); i++)
		{
			if (mask == allButtons[i].button->mask && allButtons[i].button->flags.if_hovered == 0)
			{
				allButtons[i].button->flags.if_hovered = 1;
				std::cout << mask << std::endl;
			}
			else if (mask != allButtons[i].button->mask && allButtons[i].button->flags.if_hovered == 1)
			{
				allButtons[i].button->flags.if_hovered = 0;
			}
		}
	}
	
	glStencilMask(0xFF);
	glDisable(GL_STENCIL_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}