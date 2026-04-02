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

#define A ASSETS_PATH
#define S SHADERS_PATH
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
#define INPUT_CALLED (1<<1)
#define INPUT_ACTIVE (1<<2)
#define INPUT_NOT_UPDATED (1<<3)
#define	INPUT_NEW_LETTER (1<<4)
#define INPUT_TEXT_SHIFT (1<<5)


#define TEXT_BACKSPACE 1
#define TEXT_LEFT (1<<1)
#define TEXT_RIGHT (1<<2)


#define STANDARD_RIGHT_MOUSEBUTTON_CALL standardCallBacksForBlocks::RightMouseButtonCall
#define STANDARD_LEFT_MOUSEBUTTON_HIDE standardCallBacksForBlocks::OnePressHide
#define STANDARD_VERTICAL_DISPLAY standardCallBacksForBlocks::VerticalDisplay

#define STANDARD_TEXT_STRIDE_SCALE_PARAM 1.03f

class ContextManager;
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
class ContextManager
{
private:
	
	void CreateUtilities();
	static bool CheckIfFirst(GLFWwindow* window);
	ContextManager(GLFWwindow* window);
	~ContextManager();
public:
	static Shader* globalButtonShader;
	static Shader* indivButtonShader;
	static Shader* textInputShader;
	static Shader* cursorInputShader;
	static Shader* sliderShader;
	GLFWwindow* win;
	windowData* winData;
	//globalTexMask - texture that stores masked pixels for FBO
	//globalTexColor - texture that stores colorful pixels for FBO
	//globalRenderBuffer - render buffer that stores stencil value for FBO
	unsigned int globalFBO = 0, globalTexMask = 0, globalTexColor = 0, globalRenderBuffer = 0;
	//globalButtonVAO - vertex array object to set instructions for button instancing
	//globalButtonSSBO - shader storage buffer object for storing button info needed for rendering
	unsigned int globalButtonVAO = 0, globalButtonSSBO = 0;
	unsigned int indivButtonVAO = 0, indivButtonVBO = 0, indivButtonEBO = 0;
	std::vector<Button*> Buttons;
	std::vector<BUTTON_SSBO> Buttons_SSBO_storage;
	std::vector<Slider*> Sliders;
	std::vector<InputField*> InputFields;
	
	static ContextManager* InitContext(GLFWwindow* window);
	static void ReleaseContext(ContextManager* context);

};

class Button
{
private:
	
	unsigned int texture;
	float border_size = 0.02f;
	glm::vec2 border_scale_vec;
	int index_in_array;
	glm::mat4 ScalingMatrix; //to scale in respond to button size
	static void* SSBO_map_ptr;
	void setButtonUtilities();

public:
	ContextManager* context;
	BUTTON_SSBO buttonData;
	vec2sq<float> buttonPos;
	vec2sq<float> size;
	void (*button_callback)(Button*, void*);
	void* button_callback_args;
	Shader* shader;
	BUTTON_FLAGS flags;
	Button(ContextManager* currentContext, std::string filename, vec2sq<float> position, float sizeX, float sizeY, int Mask, bool if_static, void (*buttonCallback)(Button* button, void* args),
		void* args);
	virtual ~Button();
	bool updateDataInSSBO();
	void drawButtonIndiv();
	static void drawAllButtons(ContextManager* context);
	void checkIfButtonClicked();
	void CallButton(bool if_call);
	bool state(); //returns button state (if_exec)
	void setButtonPos(vec2sq<float> Position_NDC);
	void setButtonCallPos(vec2sq<float> Call_Position, GCDenum coordFormat);
	virtual void execCommand();
	static int GetMouseMask(ContextManager* context);
};

/***********************************************************************/
/***********************************************************************/
/***********************************************************************/

class ButtonBlock
{

public:
	ContextManager* context;
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
	ButtonBlock(ContextManager* currentContext, std::vector<std::string>& filenames, std::vector<int>& Masks,
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
	Slider(ContextManager* currentContext, std::string sliderFrame_img, std::string sliderPoint_img, vec2sq<float> sliderPos, int Mask, bool if_static, void (*SliderCallback)(Button* slider, void* args), void (*SliderAppear)(Slider* slider, void* args),
		void (*SliderDisplay)(Slider* slider, void* args), void (*SliderHide)(Slider* slider, void* args), void* SliderCallback_args = NULL, 
		void* SliderAppear_args = NULL, void* SliderDisplay_args = NULL, void* SliderHide_args = NULL);
	~Slider();
	void drawSliderFrameInFBO();
	void setCallPos(vec2sq<float> CallPos);
	void sliderCall();
	void execCommand() override;
	static void drawAllSliderFrames(ContextManager* currentContext);
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
	
	vec2sq<float> pos_in_SC; //position in screen coordinates
	
	float res_text_size;
	float text_stride_scale_param;
	std::string intended_text;
	std::string printed_text;
	std::vector<int> letter_list; //list of letters
	std::vector<glm::mat4> letter_transforms;
	std::vector<vec2sq<float>> letterPos_list; //list of letters' positions
	unsigned int amount_symb_updated;
	void (*appear_func)(InputField*, void*);
	void (*display_func)(InputField*, void*);
	void (*hide_func)(InputField*, void*);
	void* appear_args;
	void* display_args;
	void* hide_args;
	static unsigned int amount;
	static FT_Library ft;
	static FT_Face ft_face;
	static unsigned int CharacterSSBO;
	static glm::vec2 scale;
	static void CreateFontsAtlas(ContextManager* context);
	void renderText();
	void drawCursor();
	void updateUBO();
	void inputOperations();
	void manualCursorDesignation();
	void textSelection();
	void drawSelectionRect();
	void activateInputField();
	void drawTextInFBO();
public:
	unsigned char inputFlags;
	vec2sq<float> callPos;
	float baseline; //Y-coordinate of a baseline for correct text allignment
	void transferAllSymbols(vec2sq<float> delta);
	InputField(ContextManager* currentContext, std::string line_filename, 
		float text_size, vec2sq<float> position, float sizeX, float sizeY, int Mask, bool if_static, void (*InputFieldAppearFunc)(InputField*, void*),
		void (*InputFieldHideFunc)(InputField*, void*), void (*InputFieldDisplayFunc)(InputField*, void*), void* InputFieldAppear_args,
		void* InputFieldHide_args, void* InputFieldDisplay_args);
	void setCallPos(vec2sq<float> Call_Position);
	static void drawAllTexts(ContextManager* currentContext);
	friend void key_callback(GLFWwindow* window, int key, int, int action, int mods);
	friend void char_callback(GLFWwindow* window, unsigned int codepoint);
};

namespace standardCallBacksForInputField
{
	struct Condition_struct
	{
		bool (*condition_func)(InputField* inputField, void* condition_args);
		void* condition_args;
		Condition_struct(bool (*CondFunc)(InputField* inputField, void* condition_args), void* CondArgs)
			: condition_func(CondFunc), condition_args(CondArgs)
		{}
	};
	void InputFieldAppear(InputField* input, void* args);
	void InputFieldHide(InputField* input, void* args);
	void InputFieldDisplay(InputField* input, void* args);
}

void drawAllElementsInFBO(ContextManager* context);
void displayButtons(ContextManager* context, Shader& interfaceShader, unsigned int scrVAO);
void checkFPS(float delta_time);