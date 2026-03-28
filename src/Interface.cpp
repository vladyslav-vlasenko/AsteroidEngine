#include "Interface.h"
std::vector<allButtons_type> allButtons;
std::map<unsigned char, Character> Characters;
unsigned int CharacterAtlasTex = 0;
vec2sq<unsigned int> CharacterAtlasSize(0, 0);
std::pair<unsigned char, bool> input_symbol; //composed of actial symbol, that is inputed and bool to handle keyboard button press 
std::pair<unsigned char, bool> special_symbol; //intended for special symbols like backspace, right/left arrows, etc.

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


//For Button Class
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/


unsigned int Button::FBO = 0;
unsigned int Button::texMask = 0;
unsigned int Button::texColor = 0;
unsigned int Button::renderBuffer = 0;
GLFWwindow* Button::win = nullptr;
windowData* Button::winData = nullptr;

void Button::createFBO(GLFWwindow* window)
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

Button::Button(GLFWwindow* window, Shader& Shader, std::string filename, vec2sq<float> position, float sizeX, float sizeY, int Mask, bool if_static, void (*buttonCallback)(Button* button, void* args),
	void* args, ButtonBlock* block, Slider* slider, InputField* input_field) : shader(&Shader), mask(Mask), button_callback(buttonCallback), button_callback_args(args)
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
		flags.if_called = true;
		buttonPos = position;
	}
	float dimX, dimY;
	if (sizeX == 0 && sizeY == 0)
	{
		dimX = 2 * width * 1.0 / winData->WIDTH;
		dimY = 2 * height * 1.0 / winData->HEIGHT;
	}
	else
	{
		dimX = 2 * sizeX * 1.0 / winData->WIDTH;
		dimY = 2 * sizeY * 1.0 / winData->HEIGHT;
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
	border_scale_vec = glm::vec2(1.0f + border_size / size.x, 1.0f + border_size / size.y);
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

Button::~Button()
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

void Button::drawInFBO(vec2sq<float> callPos = vec2sq<float>(0.0f, 0.0f))
{
	transform = glm::translate(glm::mat4(1.0f), glm::vec3(buttonPos.x, buttonPos.y, 0.0f));
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
	if (flags.if_in_slider == 1)
	{
		float callPos_y = (Button::winData->HEIGHT * 1.0f / 2) * (1 + callPos.y);
		shader->SetInt("callPos_y", callPos_y);
	}
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glStencilMask(0x00);
}

void Button::drawButtonInBlocksInFBO(vec2sq<float> callPos)
{
	transform = glm::translate(glm::mat4(1.0f), glm::vec3(buttonPos.x, buttonPos.y, 0.0f));
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
	shader->SetInt("callPosY", (int)(winData->HEIGHT / 2 + callPos.y * winData->HEIGHT / 2));
	shader->SetVec2("scale_vec", border_scale_vec);
	shader->SetMat4("transform", transform);
	shader->SetInt("Tex", 0);
	shader->SetInt("Mask", mask);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glStencilMask(0x00);
}

void Button::drawButtonBorder(vec2sq<float> callPos = vec2sq<float>(0.0f, 0.0f))
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
		shader->SetVec2("scale_vec", border_scale_vec);
		shader->SetInt("Tex", 0);
		shader->SetInt("Mask", 1);
		if (flags.if_in_slider == 1)
		{
			float callPos_y = (Button::winData->HEIGHT * 1.0f / 2) * (1 + callPos.y);
			shader->SetInt("callPos_y", callPos_y);
		}
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}

}

void Button::drawButtonInBlockBorder(vec2sq<float> callPos)
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
		shader->SetVec2("scale_vec", border_scale_vec);
		shader->SetMat4("transform", transform);
		shader->SetInt("Tex", 0);
		shader->SetInt("Mask", 1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}

}

void Button::checkIfButtonClicked()
{
	if (flags.if_checked == 1)
	{
		flags.if_checked == 0;
	}
	if (flags.if_called == 1)
	{
		if (winData->mouse_pressed == 1)
		{
			flags.if_checked = 1;
		}
		if ((flags.if_hovered == 1 && winData->mouse_pressed == 1) && flags.if_waits_to_be_pressed == 0)
		{
			flags.if_waits_to_be_pressed = 1;
			flags.if_just_pressed = 1;
		}

		else if (flags.if_just_pressed == 1)
		{
			flags.if_just_pressed = 0;
		}

		else if ((winData->mouse_pressed == 0 && flags.if_hovered == 1) && flags.if_waits_to_be_pressed == 1)
		{
			flags.if_waits_to_be_pressed = 0;
			flags.if_pressed = 1;
			
		}
		else if (flags.if_waits_to_be_pressed == 0 && flags.if_pressed != 0)
		{
			flags.if_pressed = 0;
		}
		else if (flags.if_hovered == 0 && flags.if_waits_to_be_pressed == 1)
		{
			flags.if_waits_to_be_pressed = 0;
		}
		
		
	}
	else if (flags.if_called == 0)
	{
		flags.if_hovered = 0;
		flags.if_pressed = 0;
		flags.if_checked = 0;
		flags.if_waits_to_be_pressed = 0;
	}
}


void Button::CallButton(bool if_call)
{
	if (flags.if_static == 0)
	{
		if (if_call == true)
		{
			flags.if_called = 1;
			std::cout << "TRUE" << std::endl;
		}
		else if (if_call == false)
		{
			std::cout << "FALSE" << std::endl;
			flags.if_called = 0;
			
		}
	}
}

bool Button::state()
{
	if (flags.if_exec == 1)
		return true;
	else if (flags.if_exec == 0)
		return false;
}

void Button::execCommand()
{
	
		if (flags.if_pressed == 1 && flags.if_exec == 0)
		{
			if (button_callback != NULL)
				button_callback(this, button_callback_args);
			flags.if_exec = 1;
			std::cout << "PRESSED&EXEC" << std::endl;
		}
		else if (flags.if_pressed == 0)
			flags.if_exec = 0;

}

void Button::displayButtons(Shader& interfaceShader, unsigned int scrVAO)
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

int Button::GetMouseMask()
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

void Button::drawAllElementsInFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
		{
			allButtons[i].slider->drawInFBO();
		}
		else if (allButtons[i].button->flags.if_display == 1 && allButtons[i].button->flags.if_in_inputfield == 1)
		{
			allButtons[i].button->drawInFBO();
		}
		else if (allButtons[i].button->flags.if_display == 1)
			allButtons[i].button->drawInFBO();
	}
	for (int i = 0; i < allButtons.size(); i++)
	{
		if ((allButtons[i].button->flags.if_display == 1 && allButtons[i].button->flags.if_called == 1) && allButtons[i].button->flags.if_in_block == 0)
			allButtons[i].button->drawButtonBorder();
		else if ((allButtons[i].button->flags.if_display == 1 && allButtons[i].button->flags.if_called == 1) && allButtons[i].button->flags.if_in_block == 1)
			allButtons[i].button->drawButtonInBlockBorder(allButtons[i].block->callPos);
	}
	int mask = GetMouseMask();
	if (mask != -1)
	{
		for (int i = 0; i < allButtons.size(); i++)
		{
			if (allButtons[i].button->flags.if_called == 1)
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
	}

	glStencilMask(0xFF);
	glDisable(GL_STENCIL_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


//For ButtonBlock Class
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/


ButtonBlock::ButtonBlock(GLFWwindow* window, std::vector<std::string>& filenames, Shader& shader, std::vector<int>& Masks,
	void (*CallFunctionCallback)(ButtonBlock*, void* args), void (*HideFunctionCallback)(ButtonBlock* block, void* args),
	void (*DisplayingFunctionCallback)(ButtonBlock*, void* args), std::vector<void (*)(Button*, void*)> ButtonsFunction, std::vector<void*>& buttons_function_args,
	void* call_args, void* hide_args, void* display_args)
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
		Button* but = new Button(window, shader, filenames[i], vec2sq<float>(0.0f, 0.0f), 0, 0, Masks[i], false, ButtonsFunction[i], buttons_function_args[i], this);
		buttons.push_back(but);
		but->flags.if_in_block = true;
	}
}

ButtonBlock::~ButtonBlock()
{

	for (int i = 0; i < buttons.size(); i++)
	{
		delete buttons[i];
	}
}

void ButtonBlock::blockCall()
{
	call_func(this, call_func_args);
	hide_func(this, hide_func_args);
	display_func(this, display_func_args);
}


//For Slider Class
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/


void Slider::slider_func()
{
	
	buttonPos.y = framePosition.y;
	refVector = buttonPos - framePosition;
	frameTransform = glm::translate(glm::mat4(1.0f), glm::vec3(framePosition.x, framePosition.y, 0.0f));
	if (flags.if_waits_to_be_pressed == 1)
	{
		double mouseX, mouseY;
		glfwGetCursorPos(Button::win, &mouseX, &mouseY);
		float mouseX_norm = (2 * mouseX - Button::winData->WIDTH) * 1.0f / Button::winData->WIDTH;
		check_slider_pos(mouseX_norm);
		count_result();
	}

}

void Slider::check_slider_pos(float mouseX_norm)
{
	if (mouseX_norm > (framePosition + 0.5 * (frameSize - size)).x)
		buttonPos.x = (framePosition + 0.5 * (frameSize - size)).x;
	else if (mouseX_norm < (framePosition + (-0.5) * (frameSize - size)).x)
		buttonPos.x = (framePosition + (-0.5) * (frameSize - size)).x;
	else
		buttonPos.x = mouseX_norm;
}

Slider::Slider(GLFWwindow* window, std::string sliderFrame_img, std::string sliderPoint_img, vec2sq<float> sliderPos, Shader& button_shader, 
	Shader& frame_shader, int Mask, bool if_static, void (*SliderCallback)(Button* slider, void* args), void (*SliderAppear)(Slider* slider, void* args), 
	void (*SliderDisplay)(Slider* slider, void* args), void (*SliderHide)(Slider* slider, void* args), void* SliderCallback_args, void* SliderAppear_args, 
	void* SliderDisplay_args, void* SliderHide_args) :
	Button(window, button_shader, sliderPoint_img, vec2sq<float>(0.0f, 0.0f), 0.0f, 0.0f, Mask, if_static, SliderCallback, NULL, NULL, this),
	frameShader(&frame_shader), appear_func(SliderAppear), display_func(SliderDisplay), hide_func(SliderHide), 
	appear_args(SliderAppear_args), display_args(SliderDisplay_args), hide_args(SliderHide_args)
{
	slider_value = 0.5f;
	flags.if_in_slider = 1;
	unsigned char* frameImgData;
	int frameImgWidth, frameImgHeight;
	GLenum frameImgFormat;
	load_image(sliderFrame_img, frameImgData, frameImgWidth, frameImgHeight, frameImgFormat);
	generateTexture(frameTex, frameImgData, frameImgWidth, frameImgHeight, frameImgFormat);
	stbi_image_free(frameImgData);
	float sizeX_norm = 2 * frameImgWidth * 1.0f / Button::winData->WIDTH;
	float sizeY_norm = 2 * frameImgHeight * 1.0f / Button::winData->HEIGHT;
	frameSize.x = sizeX_norm;
	frameSize.y = sizeY_norm;
	frame_border_scale_vec = glm::vec2(1.0f + frame_border_size / frameSize.x, 1.0f + frame_border_size / frameSize.y);
	if (if_static)
	{
		framePosition = sliderPos;
		buttonPos = framePosition;
	}
	
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
	
	refVector = buttonPos - framePosition;
	std::cout << frameImgWidth << std::endl;
}
Slider::~Slider()
{
	glDeleteVertexArrays(1, &frameVAO);
	glDeleteBuffers(1, &frameVBO);
	glDeleteBuffers(1, &frameEBO);
	glDeleteTextures(1, &frameTex);

}
void Slider::drawInFBO()
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
	float callPos_y = (Button::winData->HEIGHT * 1.0f / 2) * (1 + callPos.y);
	frameShader->SetFloat("callPos_y", callPos_y);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glStencilFunc(GL_NOTEQUAL, 2, 0xFF);
	glStencilMask(0x00);
	frameShader->use();
	frameShader->SetMat4("transform", frameTransform);
	frameShader->SetInt("type", 1);
	frameShader->SetVec2("scale_vec", frame_border_scale_vec);
	frameShader->SetInt("Tex", 0);
	frameShader->SetInt("Mask", 18);
	frameShader->SetFloat("callPos_y", callPos_y);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	Button::drawInFBO(callPos);
}

void Slider::sliderCall()
{
	appear_func(this, appear_args);
	display_func(this, display_args);
	hide_func(this, hide_args);
}

void Slider::execCommand()
{
	
	if (button_callback != NULL)
	{
		if (flags.if_waits_to_be_pressed == 1 && flags.if_exec == 0)
		{
			flags.if_exec = 1;
		}
		else if (flags.if_waits_to_be_pressed == 0)
			flags.if_exec = 0;
		if (flags.if_exec == 1)
		{
			button_callback(this, button_callback_args);
		}
	}

}

//For namespace
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/


void standardCallBacksForBlocks::RightMouseButtonCall(ButtonBlock* block, void*)
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
	if (right_mouse_button_state == GLFW_PRESS && ((block->flags & BUTTONBLOCK_CALLED) == 0 || (block->flags & BUTTONBLOCK_EMERGENCE_CLOSE) != 0))
	{
		if ((block->flags & BUTTONBLOCK_EMERGENCE_CLOSE) != 0)
			block->flags &= ~BUTTONBLOCK_EMERGENCE_CLOSE;
		block->flags |= BUTTONBLOCK_CALLED;
		block->flags |= BUTTONBLOCK_CALLED_FIRST;
		block->flags |= BUTTONBLOCK_RIGHT_M_BUT_FROM_THE_BEGIN;
		double posX, posY;
		glfwGetCursorPos(Button::win, &posX, &posY);
		block->callPos.x = (2 * posX - Button::winData->WIDTH) * 1.0f / Button::winData->WIDTH;
		block->callPos.y = (-2 * posY + Button::winData->HEIGHT) * 1.0f / Button::winData->HEIGHT;
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
	else if ((right_mouse_button_state == GLFW_RELEASE && (block->flags & BUTTONBLOCK_RIGHT_M_BUT_FROM_THE_BEGIN) != 0))
	{
		block->flags &= ~BUTTONBLOCK_RIGHT_M_BUT_FROM_THE_BEGIN;
	}
}

void standardCallBacksForBlocks::OnePressHide(ButtonBlock* block, void*)
{
	int left_mouse_button_state = glfwGetMouseButton(Button::win, GLFW_MOUSE_BUTTON_LEFT);
	int right_mouse_button_state = glfwGetMouseButton(Button::win, GLFW_MOUSE_BUTTON_RIGHT);
	if ((block->flags & BUTTONBLOCK_WAIT_TO_CLOSE) != 0)
	{
		for (int i = 0; i < block->buttons.size(); i++)
		{
			if (block->buttons[i]->flags.if_exec == 1) block->flags |= BUTTONBLOCK_CLOSE;
		}
	}
	
	if ((left_mouse_button_state == GLFW_PRESS && (block->flags & BUTTONBLOCK_WAIT_TO_CLOSE) == 0) && (block->flags & BUTTONBLOCK_CALLED) != 0)
	{
		for (int i = 0; i < block->buttons.size(); i++)
		{
			if (block->buttons[i]->flags.if_waits_to_be_pressed == 1)
			{
				block->flags |= BUTTONBLOCK_WAIT_TO_CLOSE;
				block->active_button_num = i;
				std::cout << "BUTTONBLOCK_WAIT_TO_CLOSE" << std::endl;
				break;
			}
		}
		if (block->active_button_num == -1 && (block->buttons[0]->flags.if_checked == 1))
		{
			block->flags |= BUTTONBLOCK_CLOSE;
		}
		
		
	}
	else if ((block->flags & BUTTONBLOCK_CLOSE) != 0)
	{
		for (int i = 0; i < block->buttons.size(); i++)
		{
			block->buttons[i]->CallButton(false);
		}
		block->flags &= ~BUTTONBLOCK_WAIT_TO_CLOSE;
		block->flags &= ~BUTTONBLOCK_CLOSE;
		block->flags &= ~BUTTONBLOCK_CALLED;
		block->active_button_num = -1;
		std::cout << "BUTTONBLOCK_CLOSED" << std::endl;
	}
	else if ((right_mouse_button_state == GLFW_PRESS && (block->flags & BUTTONBLOCK_WAIT_TO_CLOSE) == 0) && (block->flags & BUTTONBLOCK_CALLED) != 0 && (block->flags & BUTTONBLOCK_RIGHT_M_BUT_FROM_THE_BEGIN) == 0)
	{
		for (int i = 0; i < block->buttons.size(); i++)
		{
			block->buttons[i]->CallButton(false);
		}
		block->flags &= ~BUTTONBLOCK_CALLED;
		block->active_button_num = -1;
		block->flags |= BUTTONBLOCK_EMERGENCE_CLOSE;
	}
}

void standardCallBacksForBlocks::VerticalDisplay(ButtonBlock * block, void* args) //args = (std::vector<vec2sq<float>> targetButtonPoses)
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
			block->buttons[i]->buttonPos = block->callPos + vec2sq<float>(block->buttons[i]->size.x / 2, block->buttons[i]->size.y / 2);

		}
	}
	if ((block->flags & BUTTONBLOCK_CALLED) != 0)
	{

		for (int i = 0; i < block->buttons.size(); i++)
		{
			if (block->buttons[i]->buttonPos.y > block->callPos.y + casted_args->targetButtonPoses[i].y) block->buttons[i]->buttonPos.y -= 0.002f;
		}
	}
	else if ((block->flags & BUTTONBLOCK_CALLED) == 0)
	{
		if ((block->flags & BUTTONBLOCK_EMERGENCE_CLOSE) != 0)
		{
			for (int i = 0; i < block->buttons.size(); i++)
			{
				block->buttons[i]->flags.if_display = false;
			}
		}
		else
		{
			for (int i = 0; i < block->buttons.size(); i++)
			{
				if (block->buttons[i]->buttonPos.y > block->callPos.y + block->buttons[i]->size.y / 2) block->buttons[i]->flags.if_display = false;
				else if (block->buttons[i]->buttonPos.y <= block->callPos.y + block->buttons[i]->size.y / 2) block->buttons[i]->buttonPos.y += 0.002f;
			}
		}

	}

}


//For namespace standardCallBacksForSlider
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/


void standardCallBacksForSlider::SliderAppear(Slider* slider, void* args)
{
	Condition_struct* casted_args = (Condition_struct*)args;
	
	if (casted_args->condition_func(slider, casted_args->condition_args) == true && (slider->sliderFlags & SLIDER_CALLED) == 0)
	{
		slider->CallButton(true);
		slider->sliderFlags |= SLIDER_CALLED;
		slider->sliderFlags |= SLIDER_CALLED_FIRST;
		slider->flags.if_display = 1;
	}
	else if ((slider->sliderFlags & SLIDER_CALLED_FIRST) != 0)
	{
		slider->sliderFlags &= ~SLIDER_CALLED_FIRST;
	}
}

void standardCallBacksForSlider::SliderHide(Slider* slider, void* args)
{
	Condition_struct* casted_args = (Condition_struct*)args;

	if (casted_args->condition_func(slider, casted_args->condition_args) == true && (slider->sliderFlags & SLIDER_CALLED) != 0)
	{
		slider->sliderFlags &= ~SLIDER_CALLED;
		slider->CallButton(false);
	}
}

void standardCallBacksForSlider::SliderVerticalDisplay(Slider* slider, void*)
{
	if ((slider->sliderFlags & SLIDER_CALLED_FIRST) != 0)
	{
		slider->framePosition = slider->callPos + 0.5 * slider->frameSize;
		slider->buttonPos = slider->framePosition + slider->refVector;
		
	}
	if ((slider->sliderFlags & SLIDER_CALLED) != 0)
	{
		if (slider->framePosition.y > slider->callPos.y - 0.5 * slider->frameSize.y)
		{
			slider->framePosition.y -= 0.002f;
		}
	}
	else if ((slider->sliderFlags & SLIDER_CALLED) == 0)
	{
		if (slider->framePosition.y >= slider->callPos.y + 0.5 * slider->frameSize.y) slider->flags.if_display = 0;
		else if (slider->framePosition.y < slider->callPos.y + 0.5 * slider->frameSize.y) slider->framePosition.y += 0.002f;
	}
}


//For class InputField
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/


FT_Library InputField::ft = nullptr;
FT_Face InputField::ft_face = nullptr;
unsigned int InputField::CharacterSSBO = 0;
unsigned int InputField::amount = 0;
glm::vec2 InputField::scale = glm::vec2(0.0f);

void InputField::CreateFontsAtlas()
{
	if (ft == 0)
	{	
		glfwSetKeyCallback(Button::win, key_callback);
		glfwSetCharCallback(Button::win, char_callback);
		if (FT_Init_FreeType(&ft))
		{
			std::cout << "ERROR::CAN'T_INITIALIZE_FT_LIB" << std::endl;
			return;
		}
		if (FT_New_Face(ft, F "arial.ttf", 0, &ft_face))
		{
			std::cout << "ERROR::FAILED_TO_LOAD_FONT" << std::endl;
			return;
		}
		if (CharacterAtlasTex == 0)
		{
			glGenTextures(1, &CharacterAtlasTex);
			glBindTexture(GL_TEXTURE_2D, CharacterAtlasTex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		if (CharacterSSBO == 0)
		{
			glGenBuffers(1, &CharacterSSBO);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, CharacterSSBO);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CharacterSSBO);
			glBufferData(GL_SHADER_STORAGE_BUFFER, 256 * sizeof(Character_For_SSBO), nullptr, GL_STATIC_DRAW);
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		FT_Set_Pixel_Sizes(ft_face, 0, 48);
		unsigned int lineH = (ft_face->size->metrics.height >> 6);
		unsigned int AtlasWidth = 0;
		unsigned int AtlasHeight = 16 * lineH;
		for (unsigned int i = 0; i < 256; i+=16)
		{
			unsigned int row_width = 0;
			for (unsigned int j = i; j < i + 16; j++)
			{
				if (FT_Load_Char(ft_face, j, FT_LOAD_DEFAULT))
				{
					std::cout << "ERROR::DIDN'T_MANAGE_TO_LOAD_CHARACTER" << std::endl;
					return;
				}
				row_width += ft_face->glyph->metrics.horiAdvance >> 6;
			}
			AtlasWidth = max(row_width, AtlasWidth);
		}
		CharacterAtlasSize.x = AtlasWidth;
		CharacterAtlasSize.y = AtlasHeight;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, CharacterAtlasSize.x, CharacterAtlasSize.y, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
		int cur_y = 0;
		Character_For_SSBO AtlasData[256];
		for (unsigned int i = 0; i < 256; i += 16)
		{
			int cur_x = 0;
			for (unsigned int j = i; j < i + 16; j++)
			{
				
				if (FT_Load_Char(ft_face, j, FT_LOAD_RENDER))
				{
					std::cout << "ERROR::DIDN'T_MANAGE_TO_LOAD_CHARACTER" << std::endl;
					return;
				}
				unsigned int ch_w = ft_face->glyph->bitmap.width;
				unsigned int ch_h = ft_face->glyph->bitmap.rows;
				Character ch;
				ch.Bearing = glm::ivec2(ft_face->glyph->bitmap_left, ft_face->glyph->bitmap_top);
				ch.Size = glm::ivec2(ch_w, ch_h);
				ch.Advance = ft_face->glyph->metrics.horiAdvance >> 6;
				Characters.insert(std::pair<char, Character>(j, ch));
				Character_For_SSBO ch_ssbo;
				ch_ssbo.lu_uv = glm::vec2(cur_x * 1.0f / CharacterAtlasSize.x, (cur_y) * 1.0f / CharacterAtlasSize.y);
				ch_ssbo.lu_st = glm::vec2(0.0f, 0.0f);
				ch_ssbo.rl_uv = glm::vec2((cur_x + ch_w) * 1.0f / CharacterAtlasSize.x, (cur_y + ch_h) * 1.0f / CharacterAtlasSize.y);
				ch_ssbo.rl_st = glm::vec2((float)ch_w, -(float)ch_h);
				AtlasData[j] = ch_ssbo;
				glTexSubImage2D(GL_TEXTURE_2D, 0, cur_x, cur_y, ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows,
					GL_RED, GL_UNSIGNED_BYTE, ft_face->glyph->bitmap.buffer);
				cur_x += ch.Advance+2;
			}
			cur_y += lineH;
		}
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 256 * sizeof(Character_For_SSBO), AtlasData);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}
}

void InputField::renderText()
{
	glBindVertexArray(textVAO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, CharacterAtlasTex);
	textShader->use();
	textShader->SetVec2("scale", scale);
	textShader->SetInt("Mask", mask);
	textShader->SetInt("Atlas", 0);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 128);
	glBindVertexArray(0);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void InputField::drawCursor()
{
	
	cursor_intensity = (sin(6.0f * glfwGetTime())>0) ? 255.0f * sin(6.0f*glfwGetTime()) : 0;
	glBindVertexArray(cursorVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	cursor_and_selection_Shader->use();
	cursor_and_selection_Shader->SetMat4("transform", cursorTransform);
	cursor_and_selection_Shader->SetInt("type", 0);
	cursor_and_selection_Shader->SetFloat("intensity", cursor_intensity);
	cursor_and_selection_Shader->SetInt("Mask", mask);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void InputField::updateUBO()
{
	scale = glm::vec2(2.0f / Button::winData->WIDTH, 2.0f / Button::winData->HEIGHT);
	if (printed_text != intended_text)
	{
		std::cout << "NOT EQUAL" << std::endl;
		letter_list.assign(128, -1);
		float virt_cursorPos = strBegin;
		std::string::const_iterator c;
		int winWIDTH = Button::winData->WIDTH;
		int winHEIGHT = Button::winData->HEIGHT;
		unsigned int i = 0;
		for (c = intended_text.begin(); c != intended_text.end(); c++, i++)
		{
			char symbol = (char)*c;
			vec2sq<float> letter_pos = vec2sq<float>(virt_cursorPos + 2.0f * res_text_size * Characters[(int)symbol].Bearing.x / winWIDTH, baseline + 2.0f * res_text_size * Characters[(int)symbol].Bearing.y / winHEIGHT);
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(letter_pos.x, letter_pos.y, 0.0f));
			transforms[i] = transform;
			letter_list[i] = (int)symbol;
			letterPos_list[i] = letter_pos;
			virt_cursorPos += text_stride_scale_param * 2.0f * (Characters[(int)symbol].Advance) / winWIDTH;
		}
		if ((inputFlags & INPUT_FIRST_LAUNCH) != 0)
		{
			cursorPos = virt_cursorPos;
			cursorTransform = glm::translate(glm::mat4(1.0f), glm::vec3(cursorPos, buttonPos.y, 0.0f));
			inputFlags &= ~INPUT_FIRST_LAUNCH;
		}
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(float) + sizeof(glm::vec2), 128 * sizeof(glm::mat4), transforms.data());
		glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(float) + sizeof(glm::vec2) + 128 * sizeof(glm::mat4), 128 * sizeof(int), letter_list.data());
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		printed_text = intended_text;
	}
	
}

void InputField::inputOperations()
{
	if ((inputFlags & INPUT_ACTIVE) != 0 && input_symbol.second == true)
	{
		char symbol = (char)input_symbol.first;
		const char inserted[2] = {symbol, '\0'};
		intended_text.insert(cursorPos_in_text, inserted);
		input_symbol.second = false;
		cursorPos_in_text += 1;
		cursorPos += text_stride_scale_param * 2.0f * (Characters[symbol].Advance) / Button::winData->WIDTH;
		cursorTransform = glm::translate(glm::mat4(1.0f), glm::vec3(cursorPos, buttonPos.y, 0.0f));
		std::cout << symbol << std::endl;
	}
		
	else if ((inputFlags & INPUT_ACTIVE) != 0 && special_symbol.second == true)
	{
		if (special_symbol.first == TEXT_BACKSPACE)
		{
			std::string symbol = intended_text.substr(cursorPos_in_text - 1, 1);
			intended_text.erase(intended_text.begin() + cursorPos_in_text - 1);
			cursorPos_in_text -= 1;
			cursorPos -= text_stride_scale_param * 2.0f * (Characters[(int)(*symbol.begin())].Advance) / Button::winData->WIDTH;
			special_symbol.second = false;
		}

		else if (special_symbol.first == TEXT_LEFT)
		{
			std::string symbol = intended_text.substr(cursorPos_in_text - 1, 1);
			cursorPos_in_text -= 1;
			cursorPos -= text_stride_scale_param * 2.0f * (Characters[(int)(*symbol.begin())].Advance) / Button::winData->WIDTH;
			special_symbol.second = false;
		}

		else if (special_symbol.first == TEXT_RIGHT)
		{
			std::string symbol = intended_text.substr(cursorPos_in_text, 1);
			cursorPos_in_text += 1;
			cursorPos += text_stride_scale_param * 2.0f * (Characters[(int)(*symbol.begin())].Advance) / Button::winData->WIDTH;
			special_symbol.second = false;
		}
		cursorTransform = glm::translate(glm::mat4(1.0f), glm::vec3(cursorPos, buttonPos.y, 0.0f));
		
	}
}

void InputField::manualCursorDesignation()
{
	if (flags.if_hovered == 1 && flags.if_just_pressed == 1)
	{
		double mouseX_SC, mouseY_SC;
		glfwGetCursorPos(Button::win, &mouseX_SC, &mouseY_SC);
		float mouseX_NDC = 2 * mouseX_SC / Button::winData->WIDTH - 1;
		if (mouseX_NDC >= letterPos_list[printed_text.size() - 1].x + text_stride_scale_param * 2.0f * (Characters[letter_list[printed_text.size() - 1]].Advance) / Button::winData->WIDTH)
		{
			cursorPos = letterPos_list[printed_text.size() - 1].x - 2.0f * res_text_size * Characters[letter_list[printed_text.size() - 1]].Bearing.x / Button::winData->WIDTH + text_stride_scale_param * 2.0f * (Characters[letter_list[printed_text.size() - 1]].Advance) / Button::winData->WIDTH;
			cursorTransform = glm::translate(glm::mat4(1.0f), glm::vec3(cursorPos, buttonPos.y, 0.0f));
			cursorPos_in_text = printed_text.size();
		}
		else if (mouseX_NDC >= letterPos_list[printed_text.size() - 1].x)
		{
			cursorPos = letterPos_list[printed_text.size() - 1].x - 2.0f * res_text_size * Characters[letter_list[printed_text.size() - 1]].Bearing.x / Button::winData->WIDTH;
			cursorTransform = glm::translate(glm::mat4(1.0f), glm::vec3(cursorPos, buttonPos.y, 0.0f));
			cursorPos_in_text = printed_text.size() - 1;
		}
		else if (mouseX_NDC <= letterPos_list[0].x)
		{
			cursorPos = letterPos_list[0].x - 2.0f * res_text_size * Characters[letter_list[printed_text.size() - 1]].Bearing.x / Button::winData->WIDTH;
			cursorTransform = glm::translate(glm::mat4(1.0f), glm::vec3(cursorPos, buttonPos.y, 0.0f));
			cursorPos_in_text = 0;
		}
		else
		{
			for (int i = 1; i < printed_text.size(); i++)
			{

				if (mouseX_NDC >= letterPos_list[i-1].x && mouseX_NDC <= letterPos_list[i].x)
				{
					cursorPos = letterPos_list[i-1].x - 2.0f * res_text_size * Characters[letter_list[i-1]].Bearing.x / Button::winData->WIDTH;
					cursorTransform = glm::translate(glm::mat4(1.0f), glm::vec3(cursorPos, buttonPos.y, 0.0f));
					cursorPos_in_text = i - 1;
					break;
				}
			}
		}
		
	}
	
}

void InputField::textSelection()
{
	
	if (flags.if_just_pressed == 1)
	{
		virt_cursorPos = cursorPos;
		virt_cursorPos_in_text = cursorPos_in_text;
		std::cout << "SELECTION" << std::endl;
	}
	if (flags.if_waits_to_be_pressed == 1 && flags.if_hovered == 1)
	{
		double mouseX_SC, mouseY_SC;
		glfwGetCursorPos(Button::win, &mouseX_SC, &mouseY_SC);
		float mouseX_NDC = 2 * mouseX_SC / Button::winData->WIDTH - 1;
		int init_virt_cursorPos_in_text = virt_cursorPos_in_text;
		if (mouseX_NDC >= letterPos_list[printed_text.size() - 1].x + text_stride_scale_param * 2.0f * (Characters[letter_list[printed_text.size() - 1]].Advance) / Button::winData->WIDTH)
		{
			virt_cursorPos = letterPos_list[printed_text.size() - 1].x - 2.0f * res_text_size * Characters[letter_list[printed_text.size() - 1]].Bearing.x / Button::winData->WIDTH + text_stride_scale_param * 2.0f * (Characters[letter_list[printed_text.size() - 1]].Advance) / Button::winData->WIDTH;
			virt_cursorPos_in_text = printed_text.size();
		}
		else if (mouseX_NDC >= letterPos_list[printed_text.size() - 1].x)
		{
			virt_cursorPos = letterPos_list[printed_text.size() - 1].x - 2.0f * res_text_size * Characters[letter_list[printed_text.size() - 1]].Bearing.x / Button::winData->WIDTH;
			virt_cursorPos_in_text = printed_text.size() - 1;
		}
		else if (mouseX_NDC <= letterPos_list[0].x)
		{
			virt_cursorPos = letterPos_list[0].x - 2.0f * res_text_size * Characters[letter_list[printed_text.size() - 1]].Bearing.x / Button::winData->WIDTH;
			virt_cursorPos_in_text = 0;
		}
		else
		{
			for (int i = 1; i < printed_text.size(); i++)
			{

				if (mouseX_NDC >= letterPos_list[i - 1].x && mouseX_NDC <= letterPos_list[i].x)
				{
					virt_cursorPos = letterPos_list[i - 1].x - 2.0f * res_text_size * Characters[letter_list[i - 1]].Bearing.x / Button::winData->WIDTH;
					virt_cursorPos_in_text = i - 1;
					break;
				}
			}
		}
		if (init_virt_cursorPos_in_text != virt_cursorPos_in_text)
		{
			float selection_rect_vertices[8] =
			{
				cursorPos, buttonPos.y - effectiveFieldHeight / 2,
				cursorPos, buttonPos.y + effectiveFieldHeight / 2,
				virt_cursorPos, buttonPos.y + effectiveFieldHeight / 2,
				virt_cursorPos, buttonPos.y - effectiveFieldHeight / 2,
			};
			glBindBuffer(GL_ARRAY_BUFFER, selectionVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, 8 * sizeof(float), selection_rect_vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}
}

void InputField::drawSelectionRect()
{
	glBindVertexArray(selectionVAO);
	cursor_and_selection_Shader->use();
	cursor_and_selection_Shader->SetInt("type", 1);
	cursor_and_selection_Shader->SetInt("Mask", mask);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

InputField::InputField(GLFWwindow* window, Shader& Text_Shader, Shader& Cursor_Shader, Shader& Button_Shader, std::string line_filename, float text_size, vec2sq<float> position, float sizeX, float sizeY, int Mask, bool if_static)
	: Button(window, Button_Shader, line_filename, position, sizeX, sizeY, Mask, if_static, NULL, NULL, NULL, NULL, this), textShader(&Text_Shader), cursor_and_selection_Shader(&Cursor_Shader)
{
	int winHEIGHT = Button::winData->HEIGHT;
	inputFlags = 0;
	inputFlags |= (INPUT_ACTIVE | INPUT_FIRST_LAUNCH);
	intended_text = "BCDfjfldhg";
	printed_text = "";
	cursorPos_in_text = intended_text.length();
	cursor_intensity = 0;
	if (text_size == 0)
		res_text_size = 0.8f*size.y * Button::winData->HEIGHT / 96; //initial text size 48
	else
		res_text_size = text_size;
	text_stride_scale_param = STANDARD_TEXT_STRIDE_SCALE_PARAM * res_text_size;
	float button_height_SC = size.y * winData->HEIGHT / 2;
	baseline = buttonPos.y -32 * res_text_size / (winHEIGHT);
	flags.if_in_inputfield = 1;
	CreateFontsAtlas();
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBindBufferBase(GL_UNIFORM_BUFFER, amount, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(float) + sizeof(glm::vec2) + 128 * sizeof(glm::mat4) + 32 * sizeof(glm::ivec4), nullptr, GL_DYNAMIC_DRAW);
	strBegin = position.x - size.x / 2 + 0.01f * size.x;
	strEnd = position.x + size.x / 2 - 0.01f * size.x;
	effectiveFieldHeight = 0.98f * size.y;
	if (text_size == 0)
		res_text_size = 0.9f * effectiveFieldHeight * Button::winData->HEIGHT / 96; //initial text size 48
	else
		res_text_size = text_size;
	cursorPos = strBegin;
	cursorWidth = 2.0f / Button::winData->WIDTH;
	letter_list.resize(128);
	letterPos_list.resize(128);
	transforms.resize(128);
	letter_list.assign(128, -1);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float), &res_text_size);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glGenVertexArrays(1, &textVAO);
	float cursorHeight = 0.9f * effectiveFieldHeight;
	float cursor_vertices[8] =
	{
		-cursorWidth / 2, -cursorHeight / 2,
		-cursorWidth / 2, cursorHeight / 2,
		cursorWidth / 2, cursorHeight / 2,
		cursorWidth / 2, -cursorHeight / 2
	};

	unsigned int cursor_indices[6] = { 0, 1, 2,   0, 2, 3 };

	glGenVertexArrays(1, &cursorVAO);
	glGenBuffers(1, &cursorVBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(cursorVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cursorVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), cursor_vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), cursor_indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenVertexArrays(1, &selectionVAO);
	glGenBuffers(1, &selectionVBO);
	glBindVertexArray(selectionVAO);
	glBindBuffer(GL_ARRAY_BUFFER, selectionVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void InputField::drawInFBO(vec2sq<float> callPos)
{
	Button::drawInFBO(callPos);
	inputOperations();
	updateUBO();
	manualCursorDesignation();
	textSelection();
	renderText();
	drawSelectionRect();
	drawCursor();
	
}

void key_callback(GLFWwindow* window, int key, int, int action, int mods)
{
	if (key != GLFW_KEY_BACKSPACE && key != GLFW_KEY_RIGHT && key != GLFW_KEY_LEFT)
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			input_symbol.second = true;
		}
			
		else if (action == GLFW_RELEASE)
		{
			input_symbol.second = false;
		}	
	}
	
	if (key == GLFW_KEY_BACKSPACE)
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			special_symbol.first |= TEXT_BACKSPACE;
			special_symbol.second = true;
		}
		else if (action == GLFW_RELEASE)
		{
			special_symbol.first &= ~TEXT_BACKSPACE;
			special_symbol.second = false;
		}
	}
	 
	if (key == GLFW_KEY_LEFT)
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			special_symbol.first |= TEXT_LEFT;
			special_symbol.second = true;
		}
		else if (action == GLFW_RELEASE)
		{
			special_symbol.first &= ~TEXT_LEFT;
			special_symbol.second = false;
		}
	}

	if (key == GLFW_KEY_RIGHT)
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			special_symbol.first |= TEXT_RIGHT;
			special_symbol.second = true;
		}
		else if (action == GLFW_RELEASE)
		{
			special_symbol.first &= ~TEXT_RIGHT;
			special_symbol.second = false;
		}
	}
}

void char_callback(GLFWwindow* window, unsigned int codepoint)
{
	input_symbol.first = (unsigned char)codepoint;
}





