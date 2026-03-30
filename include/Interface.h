#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include "ft2build.h"
#include FT_FREETYPE_H
#include<stb_image.h>
#include<iostream>
#include<vector>
#include<cmath>
#include<thread>
#include<future>
#include<string>
#include<map>
#include "Shader.h"

#define F FONTS_PATH

#define BUTTONBLOCK_CALLED 1	
#define BUTTONBLOCK_CALLED_FIRST (1<<1)
#define BUTTONBLOCK_WAIT_TO_CLOSE (1<<2)
#define BUTTONBLOCK_CLOSE (1<<3)
#define BUTTONBLOCK_EMERGENCE_CLOSE (1<<4)
#define BUTTONBLOCK_RIGHT_M_BUT_FROM_THE_BEGIN (1<<5) //means button wasn't released any time from the button block reveal

#define SLIDER_CALLED 1
#define SLIDER_CALLED_FIRST (1<<1)
#define SLIDER_WAIT_TO_CLOSE (1<<2)

#define INPUT_FIRST_LAUNCH 1
#define INPUT_ACTIVE (1<<1)
#define INPUT_NOT_UPDATED (1<<2)
#define	INPUT_NEW_LETTER (1<<3)
#define INPUT_TEXT_SHIFT (1<<4)


#define TEXT_BACKSPACE 1
#define TEXT_LEFT (1<<1)
#define TEXT_RIGHT (1<<2)


#define STANDARD_RIGHT_MOUSEBUTTON_CALL standardCallBacksForBlocks::RightMouseButtonCall
#define STANDARD_LEFT_MOUSEBUTTON_HIDE standardCallBacksForBlocks::OnePressHide
#define STANDARD_VERTICAL_DISPLAY standardCallBacksForBlocks::VerticalDisplay

#define STANDARD_TEXT_STRIDE_SCALE_PARAM 1.03f


union BUTTON_FLAGS
{
private:
	short int flags;
public:
	struct
	{
		unsigned char if_called : 1;
		unsigned char if_checked : 1; //defines whether the button was checked after mousepress
		unsigned char if_pressed : 1; //if 1 - button was in waits_to_be_pressed state but left mouse button was released (button is hovered)
//- so buttons' funcs will start their execution
		unsigned char if_waits_to_be_pressed : 1; //if 1 - button is hovered and left mouse button is pressed (but still not released)
		unsigned char if_just_pressed : 1; //to detect the very first moment of button pressing;
		unsigned char if_hovered : 1;
		unsigned char if_static : 1;
		unsigned char if_display : 1;
		unsigned char if_exec : 1;
		unsigned char if_in_block : 1;
		unsigned char if_in_slider : 1;
		unsigned char if_in_inputfield : 1;
		unsigned char if_was_changed : 1;
		
	};
	void clear()
	{
		if_called = 0;
		if_checked = 0;
		if_pressed = 0;
		if_waits_to_be_pressed = 0;
		if_just_pressed = 0;
		if_hovered = 0;
		if_static = 0;
		if_display = 0;
		if_exec = 0;
		if_in_block = 0;
		if_in_slider = 0;
		if_in_inputfield = 0;
		if_was_changed = 0;
	}
};

struct BUTTON_SSBO
{
	float vertices[20];
	glm::mat4 transform; 
	glm::vec2 callPos;
	int mask;
	int if_global;
	GLuint64 TexHandler;
	int if_draw;
};

template <typename T>
struct vec2sq
{
	T x, y;
	vec2sq(T X, T Y) : x(X), y(Y) {}
	vec2sq() {}
	vec2sq(glm::vec2 vec);
	friend vec2sq<T> operator+(const vec2sq<T>& a, const vec2sq<T>& b)
	{
		return { a.x + b.x, a.y + b.y };
	}
	friend vec2sq<T> operator-(const vec2sq<T>& a, const vec2sq<T>& b)
	{
		return { a.x - b.x, a.y - b.y };
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
	operator glm::vec2() const
	{
		return glm::vec2((float)this->x, (float)this->y);
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

struct Character
{
	glm::ivec2 Size; 
	glm::ivec2 Bearing; 
	unsigned int Advance; 
};

__declspec(align(16)) struct Character_For_SSBO
{
	glm::vec2 lu_uv; //left upper corner UV
	glm::vec2 rl_uv; //right lower corner UV
	glm::vec2 lu_st; //left upper corner ST
	glm::vec2 rl_st; //right lower corner ST
};

enum class GCDenum
{
	GCD_NDC, //Normalized coordinates;
	GCD_SC   //Screen coordinates;
};

void mousemove_callback(GLFWwindow* window, double xpos, double ypos);
void mousebuttonclick_callback(GLFWwindow* window, int button, int action, int mods);
void load_cursor_image(std::string filename, unsigned char*& data, int& width, int& height);
void load_image(std::string filename, unsigned char*& data, int& width, int& height, GLenum& format);
void generateTexture(unsigned int& texture, unsigned char* data, int& width, int& height, GLenum format);

class Button;
class ButtonBlock;
class Slider;
class InputField;
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

class Button
{
private:
	
	unsigned int VAO, VBO, EBO, texture;
	float border_size = 0.02f;
	glm::vec2 border_scale_vec;
	int index_in_array;
	
	static void* SSBO_map_ptr;
	static void  createFBO(GLFWwindow* window);

public:
	static unsigned int FBO;
	static unsigned int texMask; //texture that stores masked pixels for FBO
	static unsigned int texColor; //texture that stores colorful pixels for FBO
	static unsigned int renderBuffer; //render buffer that stores stencil value for FBO
	static unsigned int globVAO;
	static unsigned int SSBO;
	BUTTON_SSBO buttonData;
	vec2sq<float> buttonPos;
	vec2sq<float> size;
	void (*button_callback)(Button*, void*);
	void* button_callback_args;
	Shader* shader;
	static Shader* global_shader;
	static Shader* indiv_shader;
	BUTTON_FLAGS flags;
	static GLFWwindow* win;
	static windowData* winData;
	Button(GLFWwindow* window, Shader& globalShader, Shader& indivShader, std::string filename, vec2sq<float> position, float sizeX, float sizeY, int Mask, bool if_static, void (*buttonCallback)(Button* button, void* args),
		void* args, Slider* slider = NULL, InputField* input_field = NULL);
	virtual ~Button();
	bool updateDataInSSBO();
	void drawButtonIndiv();
	static void drawAllButtons();
	void checkIfButtonClicked();
	void CallButton(bool if_call);
	bool state(); //returns button state (if_exec)
	void setButtonPos(vec2sq<float> Position_NDC);
	void setButtonCallPos(vec2sq<float> Call_Position, GCDenum coordFormat);
	virtual void execCommand();
	
	
	static int GetMouseMask();
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
	ButtonBlock(GLFWwindow* window, std::vector<std::string>& filenames, Shader& globalShader, Shader& indivShader, std::vector<int>& Masks,
		void (*CallFunctionCallback)(ButtonBlock*, void* args), void (*HideFunctionCallback)(ButtonBlock* block, void* args),
		void (*DisplayingFunctionCallback)(ButtonBlock*, void* args), std::vector<void (*)(Button*, void*)> ButtonsFunction, std::vector<void*>& buttons_function_args,
		void* call_args = NULL, void* hide_args = NULL, void* display_args = NULL);
	~ButtonBlock();
	void blockCall();
	
};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

class Slider;

namespace standardCallBacksForSlider
{
	struct Condition_struct
	{
		bool (*condition_func)(Slider* slider, void* condition_args);
		void* condition_args;
		Condition_struct(bool (*CondFunc)(Slider* slider, void* condition_args), void* CondArgs)
			: condition_func(CondFunc), condition_args(CondArgs) 
		{}
	};
	void SliderAppear(Slider* slider, void* args);
	void SliderHide(Slider* slider, void* args);
	void SliderVerticalDisplay(Slider* slider, void* args);
}


class Slider : public Button
{
private:
	unsigned int frameVAO, frameVBO, frameEBO, frameTex;
	Shader* frameShader;
	char sliderFlags;
	vec2sq<float> framePosition;
	vec2sq<float> frameSize;
	vec2sq<float> refVector;
	glm::mat4 frameTransform;
	float frame_border_size = 0.03f;
	glm::vec2 frame_border_scale_vec;
	
	void (*appear_func)(Slider*, void*);
	void (*display_func)(Slider*, void*);
	void (*hide_func)(Slider*, void*);
	void* appear_args;
	void* display_args;
	void* hide_args;
	void slider_func();
	void check_slider_pos(float mouseX_norm);
	void count_result()
	{
		slider_value = (buttonPos.x - (framePosition.x - (frameSize - size).x / 2))/(frameSize-size).x;
	}
public:
	float slider_value; //given in interval [0; 1]
	vec2sq<float> callPos;
	Slider(GLFWwindow* window, std::string sliderFrame_img, std::string sliderPoint_img, vec2sq<float> sliderPos, Shader& globalShader, Shader& indivShader,
		Shader& frame_shader, int Mask, bool if_static, void (*SliderCallback)(Button* slider, void* args), void (*SliderAppear)(Slider* slider, void* args),
		void (*SliderDisplay)(Slider* slider, void* args), void (*SliderHide)(Slider* slider, void* args), void* SliderCallback_args = NULL, 
		void* SliderAppear_args = NULL, void* SliderDisplay_args = NULL, void* SliderHide_args = NULL);
	~Slider();
	void drawSliderFrameInFBO();
	void setCallPos(vec2sq<float> CallPos);
	void sliderCall();
	void execCommand() override;
	static void drawAllSliderFrames();
	friend void standardCallBacksForSlider::SliderAppear(Slider* slider, void* args);
	friend void standardCallBacksForSlider::SliderHide(Slider* slider, void* args);
	friend void standardCallBacksForSlider::SliderVerticalDisplay(Slider* slider, void* args);
};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

namespace standardCallBacksForBlocks
{
	void RightMouseButtonCall(ButtonBlock* block, void*);
	void OnePressHide(ButtonBlock* block, void*);
	struct VerticalDisplay_args
	{
		std::vector<vec2sq<float>> targetButtonPoses;
		VerticalDisplay_args() {}
	};
	void VerticalDisplay(ButtonBlock* block, void* args); //args = (std::vector<vec2sq<float>> targetButtonPoses)
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
void key_callback(GLFWwindow*, int, int, int, int);
void char_callback(GLFWwindow*, unsigned int);
class InputField : public Button
{
private:
	unsigned int textVAO, UBO, cursorVAO, cursorVBO, EBO, selectionVAO, selectionVBO;
	Shader* textShader;
	Shader* cursor_and_selection_Shader;
	float strBegin; //X-coordinate of the begining of the str
	float strEnd; //X-coordinate of the end of the str
	float effectiveFieldHeight;
	float cursorPos; //X-coordinate of a cursor position
	int cursorPos_in_text;
	glm::mat4 cursorTransform;
	float cursorWidth;
	float cursor_intensity;
	float virt_cursorPos; //NDC position of virtual cursor intended for text selection
	int virt_cursorPos_in_text; //position of virtual cursor in text
	float baseline; //Y-coordinate of a baseline for correct text allignment
	vec2sq<float> pos_in_SC; //position in screen coordinates
	unsigned char inputFlags;
	float res_text_size;
	float text_stride_scale_param;
	std::string intended_text;
	std::string printed_text;
	std::vector<int> letter_list; //list of letters
	std::vector<glm::mat4> transforms;
	std::vector<vec2sq<float>> letterPos_list; //list of letters' positions
	unsigned int amount_symb_updated;
	static unsigned int amount;
	static FT_Library ft;
	static FT_Face ft_face;
	static unsigned int CharacterSSBO;
	static glm::vec2 scale;
	static void CreateFontsAtlas();
	void renderText();
	void drawCursor();
	void updateUBO();
	void inputOperations();
	void manualCursorDesignation();
	void textSelection();
	void drawSelectionRect();
	void drawTextInFBO();
public:
	InputField(GLFWwindow* window, Shader& Text_Shader, Shader& Cursor_Shader, Shader& globalShader, Shader& indivShader, std::string line_filename, float text_size, vec2sq<float> position, float sizeX, float sizeY, int Mask, bool if_static);
	static void drawAllTexts();
	friend void key_callback(GLFWwindow* window, int key, int, int action, int mods);
	friend void char_callback(GLFWwindow* window, unsigned int codepoint);
};

void drawAllElementsInFBO();
void displayButtons(Shader& interfaceShader, unsigned int scrVAO);