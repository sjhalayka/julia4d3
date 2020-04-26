#ifndef ssao_h
#define ssao_h



// This is the copyright notice that came with the SSAO code:
/*
 * Copyright � 2012-2015 Graham Sellers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#define _CRT_SECURE_NO_WARNINGS


#include <GL/glew.h>
#include <GL/glut.h>

 // Automatically link in the GLUT and GLEW libraries if compiling on MSVC++
#ifdef _MSC_VER
#pragma comment(lib, "glew32")
#pragma comment(lib, "freeglut")
#endif

#include <atomic>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <string>
#include <iostream>
#include <chrono>
#include <sstream>
using namespace std;

#include "vertex_fragment_shader.h"
#include "matrix_utils.h"
#include "uv_camera.h"
#include "mesh.h"
#include "glui/glui.h"
#include "string_utilities.h"
using namespace string_utilities;
#include "eqparse.h"
#include "marching_cubes.h"
using namespace marching_cubes;
#include "js_state_machine.h"


#include "logging_system.h"
#include "bmp.h"
#include "fractal_set_parameters.h"

fractal_set_parameters p;
js_state_machine jsm;
logging_system log_system;

std::chrono::high_resolution_clock::time_point start_time, end_time;

GLint win_id = 0;
GLuint win_x = 800;
GLuint win_y = 600;
bool lmb_down = false;
bool mmb_down = false;
bool rmb_down = false;
GLuint mouse_x = 0;
GLuint mouse_y = 0;
float u_spacer = 0.01f;
float v_spacer = 0.5f * u_spacer;
float w_spacer = 0.1f;
uv_camera main_camera;

GLUI* glui, * glui2;

GLUI_Panel* obj_panel, * obj_panel2, * obj_panel3;

GLUI_Button* generate_mesh_button, * export_to_stl_button;

GLUI_Checkbox* randomize_c_checkbox, * use_pedestal_checkbox;
GLUI_Checkbox* draw_console_checkbox;
GLUI_Checkbox* draw_axis_checkbox;


GLUI_EditText* pedestal_y_start_edittext;
GLUI_EditText* pedestal_y_end_edittext;

GLUI_EditText* equation_edittext;

GLUI_EditText* c_x_edittext;
GLUI_EditText* c_y_edittext;
GLUI_EditText* c_z_edittext;
GLUI_EditText* c_w_edittext;

GLUI_EditText* x_min_edittext;
GLUI_EditText* y_min_edittext;
GLUI_EditText* z_min_edittext;

GLUI_EditText* x_max_edittext;
GLUI_EditText* y_max_edittext;
GLUI_EditText* z_max_edittext;

GLUI_EditText* z_w_edittext;
GLUI_EditText* iterations_edittext;
GLUI_EditText* resolution_edittext;
GLUI_EditText* infinity_edittext;

GLUI_StaticText* status;




GLuint      render_fbo = 0;
GLuint      fbo_textures[3] = { 0, 0, 0 };
GLuint      quad_vao = 0;
GLuint      points_buffer = 0;




bool generate_button = true;
unsigned int triangle_buffer = 0;
unsigned int axis_buffer = 0;


class RGB
{
public:
	unsigned char r, g, b;
};









bool obtain_control_contents(fractal_set_parameters& p)
{
	ostringstream oss;

	if (p.randomize_c = randomize_c_checkbox->get_int_val())
	{
		float c_x = rand() / static_cast<float>(RAND_MAX);
		float c_y = rand() / static_cast<float>(RAND_MAX);
		float c_z = rand() / static_cast<float>(RAND_MAX);
		float c_w = rand() / static_cast<float>(RAND_MAX);

		if (rand() % 2 == 0)
			c_x = -c_x;

		if (rand() % 2 == 0)
			c_y = -c_y;

		if (rand() % 2 == 0)
			c_z = -c_z;

		if (rand() % 2 == 0)
			c_w = -c_w;

		oss.clear();
		oss.str("");
		oss << c_x;
		c_x_edittext->set_text(oss.str());

		oss.clear();
		oss.str("");
		oss << c_y;
		c_y_edittext->set_text(oss.str());

		oss.clear();
		oss.str("");
		oss << c_z;
		c_z_edittext->set_text(oss.str());

		oss.clear();
		oss.str("");
		oss << c_w;
		c_w_edittext->set_text(oss.str());
	}


	p.equation_text = equation_edittext->text;

	if (p.equation_text == "")
	{
		oss.clear();
		oss.str("");
		oss << "blank equation text";

		log_system.add_string_to_contents(oss.str());


		return false;
	}

	p.randomize_c = randomize_c_checkbox->get_int_val();
	p.use_pedestal = use_pedestal_checkbox->get_int_val();

	string temp_string;

	temp_string = pedestal_y_start_edittext->text;

	if (false == is_real_number(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "Pedestal y start is not a real number";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.pedestal_y_start;
	}

	temp_string = pedestal_y_end_edittext->text;

	if (false == is_real_number(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "Pedestal y end is not a real number";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.pedestal_y_end;
	}

	if (p.pedestal_y_start < 0 || p.pedestal_y_start > 1)
	{
		oss.clear();
		oss.str("");
		oss << "Pedestal y start must be between 0 and 1";

		log_system.add_string_to_contents(oss.str());


		return false;
	}

	if (p.pedestal_y_end < 0 || p.pedestal_y_end > 1)
	{
		oss.clear();
		oss.str("");
		oss << "Pedestal y end must be between 0 and 1";

		log_system.add_string_to_contents(oss.str());


		return false;
	}

	if (p.pedestal_y_start >= p.pedestal_y_end)
	{
		oss.clear();
		oss.str("");
		oss << "Pedestal y start must be smaller than pedestal y end";

		log_system.add_string_to_contents(oss.str());


		return false;
	}




	temp_string = c_x_edittext->text;

	if (false == is_real_number(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "C.x is not a real number";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.C_x;
	}

	temp_string = c_y_edittext->text;

	if (false == is_real_number(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "C.y is not a real number";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.C_y;
	}

	temp_string = c_z_edittext->text;

	if (false == is_real_number(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "C.z is not a real number";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.C_z;
	}

	temp_string = c_w_edittext->text;

	if (false == is_real_number(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "C.w is not a real number";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.C_w;
	}

	temp_string = x_min_edittext->text;

	if (false == is_real_number(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "x min is not a real number";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.x_min;
	}

	temp_string = y_min_edittext->text;

	if (false == is_real_number(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "y min is not a real number";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.y_min;
	}

	temp_string = z_min_edittext->text;

	if (false == is_real_number(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "z min is not a real number";

		log_system.add_string_to_contents(oss.str());

		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.z_min;
	}




	temp_string = x_max_edittext->text;

	if (false == is_real_number(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "x max is not a real number";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.x_max;
	}

	temp_string = y_max_edittext->text;

	if (false == is_real_number(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "y max is not a real number";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.y_max;
	}

	temp_string = z_max_edittext->text;

	if (false == is_real_number(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "z max is not a real number";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.z_max;
	}

	if (p.x_min >= p.x_max)
	{
		oss.clear();
		oss.str("");
		oss << "x min must be less than x max";

		log_system.add_string_to_contents(oss.str());


		return false;
	}

	if (p.y_min >= p.y_max)
	{
		oss.clear();
		oss.str("");
		oss << "y min must be less than y max";

		log_system.add_string_to_contents(oss.str());


		return false;
	}

	if (p.z_min >= p.z_max)
	{
		oss.clear();
		oss.str("");
		oss << "z min must be less than z max";

		log_system.add_string_to_contents(oss.str());


		return false;
	}

	temp_string = z_w_edittext->text;

	if (false == is_real_number(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "Z.w is not a real number";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.Z_w;
	}


	temp_string = infinity_edittext->text;

	if (false == is_real_number(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "Infinity is not a real number";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.infinity;
	}


	temp_string = iterations_edittext->text;

	if (false == is_unsigned_int(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "Max iterations is not an unsigned int";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.max_iterations;
	}

	temp_string = resolution_edittext->text;

	if (false == is_unsigned_int(temp_string))
	{
		oss.clear();
		oss.str("");
		oss << "Resolution is not an unsigned int";

		log_system.add_string_to_contents(oss.str());


		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.resolution;

		if (p.resolution < 3)
		{
			oss.clear();
			oss.str("");
			oss << "Resolution must be greater than or equal to 3";

			log_system.add_string_to_contents(oss.str());


			return false;
		}
	}

	return true;
}



void generate_cancel_button_func(int control)
{
	if (generate_button == false)
	{
		ostringstream oss;

		oss.clear();
		oss.str("");
		oss << "Aborting";

		log_system.add_string_to_contents(oss.str());

		jsm.cancel();

		generate_button = true;
		generate_mesh_button->set_name(const_cast<char*>("Generate mesh"));
	}
	else
	{
		if (false == obtain_control_contents(p))
		{
			ostringstream oss;

			oss.clear();
			oss.str("");
			oss << "Aborting";

			log_system.add_string_to_contents(oss.str());

			return;
		}

		if (false == jsm.init(p, &log_system))
		{
			ostringstream oss;

			oss.clear();
			oss.str("");
			oss << "Aborting";

			log_system.add_string_to_contents(oss.str());

			return;
		}

		generate_button = false;
		generate_mesh_button->set_name(const_cast<char*>("Cancel"));

		start_time = std::chrono::high_resolution_clock::now();

	}
}


void control_cb(int control)
{

}

void myGlutReshape(int x, int y)
{
	win_x = x;
	win_y = y;

	if (win_x < 1)
		win_x = 1;

	if (win_y < 1)
		win_y = 1;

	glutSetWindow(win_id);
	glutReshapeWindow(win_x, win_y);
	glViewport(0, 0, win_x, win_y);

	glutPostRedisplay();
}

void myGlutIdle(void)
{
	glutSetWindow(win_id);

	if ((STATE_FINISHED == jsm.get_state() || STATE_UNINITIALIZED == jsm.get_state())
		&& false == generate_button)
	{
		generate_button = true;
		generate_mesh_button->set_name(const_cast<char*>("Generate mesh"));

		end_time = std::chrono::high_resolution_clock::now();

		std::chrono::duration<float, std::milli> elapsed = end_time - start_time;

		ostringstream oss;
		oss.clear();
		oss.str("");
		oss << "Duration: " << elapsed.count() / 1000.0f << " seconds";

		log_system.add_string_to_contents(oss.str());
	}


	size_t state = jsm.get_state();

	if (STATE_FINISHED != state &&
		STATE_CANCELLED != state &&
		STATE_UNINITIALIZED != state)
	{
		std::chrono::high_resolution_clock::time_point compute_start_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float, std::milli> elapsed;

		do
		{
			jsm.proceed();

			std::chrono::high_resolution_clock::time_point compute_end_time = std::chrono::high_resolution_clock::now();
			elapsed = compute_end_time - compute_start_time;
		} while (elapsed.count() < 333); // Lower this amount to get more responsiveness
	}

	glutPostRedisplay();
}

void passive_motion_func(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
}


vertex_fragment_shader render;
vertex_fragment_shader ssao;
vertex_fragment_shader flat;





struct
{
	struct
	{
		GLint           mv_matrix;
		GLint           proj_matrix;
		GLint           shading_level;
	} render;
	struct
	{
		GLint           ssao_level;
		GLint           object_level;
		GLint           ssao_radius;
		GLint           weight_by_angle;
		GLint           randomize_points;
		GLint           point_count;
	} ssao;
	struct
	{
		GLint           mv_matrix;
		GLint           proj_matrix;
		GLint			flat_colour;
	} flat;
} uniforms;

bool  show_shading;
bool  show_ao;
float ssao_level;
float ssao_radius;
bool  weight_by_angle;
bool randomize_points;
unsigned int point_count;


struct SAMPLE_POINTS
{
	vertex_4 point[256];
	vertex_4 random_vectors[256];
};


static unsigned int seed = 0x13371337;

static inline float random_float()
{
	float res;
	unsigned int tmp;

	seed *= 16807;

	tmp = seed ^ (seed >> 4) ^ (seed << 15);

	*((unsigned int*)&res) = (tmp >> 9) | 0x3F800000;

	return (res - 1.0f);
}


bool load_shaders(void)
{
	// Set up shader
	if (false == render.init("render.vs.glsl", "render.fs.glsl"))
	{
		cout << "Could not load render shader" << endl;
		return false;
	}

	if (false == flat.init("flat.vs.glsl", "flat.fs.glsl"))
	{
		cout << "Could not load flat shader" << endl;
		return false;
	}

	// Set up shader
	if (false == ssao.init("ssao.vs.glsl", "ssao.fs.glsl"))
	{
		cout << "Could not load SSAO shader" << endl;
		return false;
	}

	uniforms.render.mv_matrix = glGetUniformLocation(render.get_program(), "mv_matrix");
	uniforms.render.proj_matrix = glGetUniformLocation(render.get_program(), "proj_matrix");
	uniforms.render.shading_level = glGetUniformLocation(render.get_program(), "shading_level");

	uniforms.ssao.ssao_radius = glGetUniformLocation(ssao.get_program(), "ssao_radius");
	uniforms.ssao.ssao_level = glGetUniformLocation(ssao.get_program(), "ssao_level");
	uniforms.ssao.object_level = glGetUniformLocation(ssao.get_program(), "object_level");
	uniforms.ssao.weight_by_angle = glGetUniformLocation(ssao.get_program(), "weight_by_angle");
	uniforms.ssao.randomize_points = glGetUniformLocation(ssao.get_program(), "randomize_points");
	uniforms.ssao.point_count = glGetUniformLocation(ssao.get_program(), "point_count");

	uniforms.flat.mv_matrix = glGetUniformLocation(flat.get_program(), "mv_matrix");
	uniforms.flat.proj_matrix = glGetUniformLocation(flat.get_program(), "proj_matrix");
	uniforms.flat.flat_colour = glGetUniformLocation(flat.get_program(), "flat_colour");

	return true;
}




class monochrome_image
{
public:
	size_t width;
	size_t height;
	vector<unsigned char> pixel_data;
};

vector<monochrome_image> mimgs;

const size_t num_chars = 256;
const size_t image_width = 256;
const size_t image_height = 256;
const size_t char_width = 16;
const size_t char_height = 16;
const size_t num_chars_wide = image_width / char_width;
const size_t num_chars_high = image_height / char_height;




void print_char(vector<unsigned char>& fbpixels, const size_t fb_width, const size_t fb_height, const size_t char_x_pos, const size_t char_y_pos, const unsigned char c, const RGB& text_colour)
{
	for (size_t i = 0; i < mimgs[c].width; i++)
	{
		for (size_t j = 0; j < mimgs[c].height; j++)
		{
			size_t y = mimgs[c].height - j;

			size_t fb_x = char_x_pos + i;
			size_t fb_y = fb_height - char_y_pos + y;

			// If out of bounds, skip this pixel
			if (fb_x >= fb_width || fb_y >= fb_height)
				continue;

			size_t fb_index = 4 * (fb_y * fb_width + fb_x);
			size_t img_index = j * mimgs[c].width + i;

			RGB background_colour;
			background_colour.r = fbpixels[fb_index + 0];
			background_colour.g = fbpixels[fb_index + 1];
			background_colour.b = fbpixels[fb_index + 2];

			const unsigned char alpha = mimgs[c].pixel_data[img_index];
			const float alpha_float = alpha / 255.0f;

			RGB target_colour;
			target_colour.r = static_cast<unsigned char>(alpha_float * double(text_colour.r - background_colour.r) + background_colour.r);
			target_colour.g = static_cast<unsigned char>(alpha_float * double(text_colour.g - background_colour.g) + background_colour.g);
			target_colour.b = static_cast<unsigned char>(alpha_float * double(text_colour.b - background_colour.b) + background_colour.b);

			fbpixels[fb_index + 0] = target_colour.r;
			fbpixels[fb_index + 1] = target_colour.g;
			fbpixels[fb_index + 2] = target_colour.b;
			fbpixels[fb_index + 3] = 255;
		}
	}
}

void print_sentence(vector<unsigned char>& fbpixels, const size_t fb_width, const size_t fb_height, size_t char_x_pos, const size_t char_y_pos, const string s, const RGB& text_colour)
{
	for (size_t i = 0; i < s.size(); i++)
	{
		print_char(fbpixels, fb_width, fb_height, char_x_pos, char_y_pos, s[i], text_colour);

		size_t char_width = mimgs[s[i]].width;

		char_x_pos += char_width + 2;
	}
}


bool is_all_zeroes(size_t width, size_t height, const vector<unsigned char>& pixel_data)
{
	bool all_zeroes = true;

	for (size_t i = 0; i < width * height; i++)
	{
		if (pixel_data[i] != 0)
		{
			all_zeroes = false;
			break;
		}
	}

	return all_zeroes;
}

bool is_column_all_zeroes(size_t column, size_t width, size_t height, const vector<unsigned char>& pixel_data)
{
	bool all_zeroes = true;

	for (size_t y = 0; y < height; y++)
	{
		size_t index = y * width + column;

		if (pixel_data[index] != 0)
		{
			all_zeroes = false;
			break;
		}
	}

	return all_zeroes;
}


bool init(void)
{
	if (GLEW_OK != glewInit())
	{
		cout << "GLEW initialization error" << endl;
		return false;
	}

	int GL_major_version = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &GL_major_version);

	int GL_minor_version = 0;
	glGetIntegerv(GL_MINOR_VERSION, &GL_minor_version);

	if (GL_major_version < 4)
	{
		cout << "GPU does not support OpenGL 4.x or higher" << endl;
		return false;
	}
	else if (GL_major_version == 4)
	{
		if (GL_minor_version < 3)
		{
			cout << "GPU does not support OpenGL 4.3 or higher" << endl;
			return false;
		}
	}

	cout << "OpenGL Version: " << GL_major_version << "." << GL_minor_version << endl;

	BMP font;

	if (false == font.load("font.bmp"))
	{
		cout << "could not load font.bmp" << endl;
		return false;
	}

	glGenBuffers(1, &triangle_buffer);
	glGenBuffers(1, &axis_buffer);

	size_t char_index = 0;

	vector< vector<GLubyte> > char_data;
	vector<unsigned char> char_template(char_width * char_height);
	char_data.resize(num_chars, char_template);

	for (size_t i = 0; i < num_chars_wide; i++)
	{
		for (size_t j = 0; j < num_chars_high; j++)
		{
			size_t left = i * char_width;
			size_t right = left + char_width - 1;
			size_t top = j * char_height;
			size_t bottom = top + char_height - 1;

			for (size_t k = left, x = 0; k <= right; k++, x++)
			{
				for (size_t l = top, y = 0; l <= bottom; l++, y++)
				{
					size_t img_pos = 4 * (k * image_height + l);
					size_t sub_pos = x * char_height + y;

					char_data[char_index][sub_pos] = font.Pixels[img_pos]; // Assume grayscale, only use r component
				}
			}

			char_index++;
		}
	}

	for (size_t n = 0; n < num_chars; n++)
	{
		if (is_all_zeroes(char_width, char_height, char_data[n]))
		{
			monochrome_image img;

			img.width = char_width / 4;
			img.height = char_height;

			img.pixel_data.resize(img.width * img.height, 0);

			mimgs.push_back(img);
		}
		else
		{
			size_t first_non_zeroes_column = 0;
			size_t last_non_zeroes_column = char_width - 1;

			for (size_t x = 0; x < char_width; x++)
			{
				bool all_zeroes = is_column_all_zeroes(x, char_width, char_height, char_data[n]);

				if (false == all_zeroes)
				{
					first_non_zeroes_column = x;
					break;
				}
			}

			for (size_t x = first_non_zeroes_column + 1; x < char_width; x++)
			{
				bool all_zeroes = is_column_all_zeroes(x, char_width, char_height, char_data[n]);

				if (false == all_zeroes)
				{
					last_non_zeroes_column = x;
				}
			}

			size_t cropped_width = last_non_zeroes_column - first_non_zeroes_column + 1;

			monochrome_image img;
			img.width = cropped_width;
			img.height = char_height;
			img.pixel_data.resize(img.width * img.height, 0);

			for (size_t i = 0; i < num_chars_wide; i++)
			{
				for (size_t j = 0; j < num_chars_high; j++)
				{
					const size_t left = first_non_zeroes_column;
					const size_t right = left + cropped_width - 1;
					const size_t top = 0;
					const size_t bottom = char_height - 1;

					for (size_t k = left, x = 0; k <= right; k++, x++)
					{
						for (size_t l = top, y = 0; l <= bottom; l++, y++)
						{
							const size_t img_pos = l * char_width + k;
							const size_t sub_pos = y * cropped_width + x;

							img.pixel_data[sub_pos] = char_data[n][img_pos];
						}
					}
				}
			}

			mimgs.push_back(img);
		}
	}

	ssao_level = 1.0f;
	ssao_radius = 0.05f;
	show_shading = true;
	show_ao = true;
	weight_by_angle = true;
	randomize_points = true;
	point_count = 10;

	if (false == load_shaders())
		return false;

	glGenFramebuffers(1, &render_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
	glGenTextures(3, fbo_textures);

	glBindTexture(GL_TEXTURE_2D, fbo_textures[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, 2048, 2048);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, fbo_textures[1]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 2048, 2048);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, fbo_textures[2]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, 2048, 2048);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_textures[0], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, fbo_textures[1], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fbo_textures[2], 0);

	static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	glDrawBuffers(2, draw_buffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &quad_vao);
	glBindVertexArray(quad_vao);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	int i;
	SAMPLE_POINTS point_data;

	for (i = 0; i < 256; i++)
	{
		do
		{
			point_data.point[i].x = random_float() * 2.0f - 1.0f;
			point_data.point[i].y = random_float() * 2.0f - 1.0f;
			point_data.point[i].z = random_float(); //  * 2.0f - 1.0f;
			point_data.point[i].w = 0.0f;
		} while (length(point_data.point[i]) > 1.0f);
		normalize(point_data.point[i]);
	}
	for (i = 0; i < 256; i++)
	{
		point_data.random_vectors[i].x = random_float();
		point_data.random_vectors[i].y = random_float();
		point_data.random_vectors[i].z = random_float();
		point_data.random_vectors[i].w = random_float();
	}

	glGenBuffers(1, &points_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, points_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SAMPLE_POINTS), &point_data, GL_STATIC_DRAW);

	return true;
}


void blur_image(vector<unsigned char>& write_p, GLuint width, GLuint height, size_t num_channels, size_t num_iterations)
{
	for (size_t n = 0; n < num_iterations; n++)
	{
		const vector<unsigned char> read_p = write_p;

		for (size_t i = 1; i < (width - 1); i++)
		{
			for (size_t j = 1; j < (height - 1); j++)
			{
				size_t centre_index = num_channels * (j * width + i);

				size_t up_index = num_channels * ((j + 1) * width + i);
				size_t down_index = num_channels * ((j - 1) * width + i);
				size_t left_index = num_channels * (j * width + (i + 1));
				size_t right_index = num_channels * (j * width + (i - 1));

				float r = 0, g = 0, b = 0, a = 0;

				r = static_cast<float>(read_p[centre_index]) + static_cast<float>(read_p[up_index]) + static_cast<float>(read_p[down_index]) + static_cast<float>(read_p[left_index]) + static_cast<float>(read_p[right_index]);
				r /= 5.0;

				g = static_cast<float>(read_p[centre_index + 1]) + static_cast<float>(read_p[up_index + 1]) + static_cast<float>(read_p[down_index + 1]) + static_cast<float>(read_p[left_index + 1]) + static_cast<float>(read_p[right_index + 1]);
				g /= 5.0;

				b = static_cast<float>(read_p[centre_index + 2]) + static_cast<float>(read_p[up_index + 2]) + static_cast<float>(read_p[down_index + 2]) + static_cast<float>(read_p[left_index + 2]) + static_cast<float>(read_p[right_index + 2]);
				b /= 5.0;

				a = 255.0f;

				write_p[centre_index + 0] = static_cast<unsigned char>(r);
				write_p[centre_index + 1] = static_cast<unsigned char>(g);
				write_p[centre_index + 2] = static_cast<unsigned char>(b);
				write_p[centre_index + 3] = static_cast<unsigned char>(a);
			}
		}
	}
}


void display_func(void)
{


	glClearColor(1, 0.5f, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_BLEND);

	const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const GLfloat one = 1.0f;
	static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	glViewport(0, 0, win_x, win_y);

	glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
	glEnable(GL_DEPTH_TEST);

	glClearBufferfv(GL_COLOR, 0, black);
	glClearBufferfv(GL_COLOR, 1, black);
	glClearBufferfv(GL_DEPTH, 0, &one);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, points_buffer);

	glUseProgram(render.get_program());

	main_camera.calculate_camera_matrices(win_x, win_y);
	glUniformMatrix4fv(uniforms.render.proj_matrix, 1, GL_FALSE, main_camera.projection_mat);
	glUniformMatrix4fv(uniforms.render.mv_matrix, 1, GL_FALSE, main_camera.view_mat);
	glUniform1f(uniforms.render.shading_level, show_shading ? (show_ao ? 0.7f : 1.0f) : 0.0f);

	if (draw_axis_checkbox->get_int_val())
	{
		glLineWidth(2.0);

		glUseProgram(flat.get_program());

		main_camera.calculate_camera_matrices(win_x, win_y);
		glUniformMatrix4fv(uniforms.flat.proj_matrix, 1, GL_FALSE, main_camera.projection_mat);
		glUniformMatrix4fv(uniforms.flat.mv_matrix, 1, GL_FALSE, main_camera.view_mat);

		const GLuint components_per_vertex = 3;
		const GLuint components_per_position = 3;

		vector<GLfloat> flat_data;

		//flat_data.clear();

		flat_data.push_back(0);
		flat_data.push_back(0);
		flat_data.push_back(0);
		flat_data.push_back(1);
		flat_data.push_back(0);
		flat_data.push_back(0);

		glUniform3f(uniforms.flat.flat_colour, 1.0, 0.0, 0.0);

		glDeleteBuffers(1, &axis_buffer);
		glGenBuffers(1, &axis_buffer);

		GLuint num_vertices = static_cast<GLuint>(flat_data.size()) / components_per_vertex;

		glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
		glBufferData(GL_ARRAY_BUFFER, flat_data.size() * sizeof(GLfloat), &flat_data[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(glGetAttribLocation(render.get_program(), "position"));
		glVertexAttribPointer(glGetAttribLocation(render.get_program(), "position"),
			components_per_position,
			GL_FLOAT,
			GL_FALSE,
			components_per_vertex * sizeof(GLfloat),
			NULL);

		glDrawArrays(GL_LINES, 0, num_vertices);

		flat_data.clear();

		flat_data.push_back(0);
		flat_data.push_back(0);
		flat_data.push_back(0);
		flat_data.push_back(0);
		flat_data.push_back(1);
		flat_data.push_back(0);

		glUniform3f(uniforms.flat.flat_colour, 0.0, 1.0, 0.0);

		glDeleteBuffers(1, &axis_buffer);
		glGenBuffers(1, &axis_buffer);

		num_vertices = static_cast<GLuint>(flat_data.size()) / components_per_vertex;

		glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
		glBufferData(GL_ARRAY_BUFFER, flat_data.size() * sizeof(GLfloat), &flat_data[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(glGetAttribLocation(render.get_program(), "position"));
		glVertexAttribPointer(glGetAttribLocation(render.get_program(), "position"),
			components_per_position,
			GL_FLOAT,
			GL_FALSE,
			components_per_vertex * sizeof(GLfloat),
			NULL);

		glDrawArrays(GL_LINES, 0, num_vertices);

		flat_data.clear();

		flat_data.push_back(0);
		flat_data.push_back(0);
		flat_data.push_back(0);
		flat_data.push_back(0);
		flat_data.push_back(0);
		flat_data.push_back(1);

		glUniform3f(uniforms.flat.flat_colour, 0.0, 0.0, 1.0);

		glDeleteBuffers(1, &axis_buffer);
		glGenBuffers(1, &axis_buffer);

		num_vertices = static_cast<GLuint>(flat_data.size()) / components_per_vertex;

		glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
		glBufferData(GL_ARRAY_BUFFER, flat_data.size() * sizeof(GLfloat), &flat_data[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(glGetAttribLocation(render.get_program(), "position"));
		glVertexAttribPointer(glGetAttribLocation(render.get_program(), "position"),
			components_per_position,
			GL_FLOAT,
			GL_FALSE,
			components_per_vertex * sizeof(GLfloat),
			NULL);

		glDrawArrays(GL_LINES, 0, num_vertices);
	}


	if (STATE_FINISHED == jsm.get_state() && jsm.vertex_data.size() > 0)
	{
		glUseProgram(render.get_program());

		const GLuint components_per_vertex = 9;
		const GLuint components_per_normal = 3;
		const GLuint components_per_position = 3;
		const GLuint components_per_colour = 3;

		glDeleteBuffers(1, &triangle_buffer);
		glGenBuffers(1, &triangle_buffer);

		const GLuint num_vertices = static_cast<GLuint>(jsm.vertex_data.size()) / components_per_vertex;

		glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer);
		glBufferData(GL_ARRAY_BUFFER, jsm.vertex_data.size() * sizeof(GLfloat), &jsm.vertex_data[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(glGetAttribLocation(render.get_program(), "position"));
		glVertexAttribPointer(glGetAttribLocation(render.get_program(), "position"),
			components_per_position,
			GL_FLOAT,
			GL_FALSE,
			components_per_vertex * sizeof(GLfloat),
			NULL);

		glEnableVertexAttribArray(glGetAttribLocation(render.get_program(), "normal"));
		glVertexAttribPointer(glGetAttribLocation(render.get_program(), "normal"),
			components_per_normal,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(components_per_position * sizeof(GLfloat)));

		glEnableVertexAttribArray(glGetAttribLocation(render.get_program(), "colour"));
		glVertexAttribPointer(glGetAttribLocation(render.get_program(), "colour"),
			components_per_colour,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(components_per_normal * sizeof(GLfloat) + components_per_position * sizeof(GLfloat)));

		// Draw 12 vertices per card
		glDrawArrays(GL_TRIANGLES, 0, num_vertices);
	}

	vector<float> depth_pixels(static_cast<size_t>(win_x)* static_cast<size_t>(win_y), 0.0f);
	glReadPixels(0, 0, win_x, win_y, GL_DEPTH_COMPONENT, GL_FLOAT, &depth_pixels[0]);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(ssao.get_program());

	glUniform1f(uniforms.ssao.ssao_radius, ssao_radius * float(win_x) / 1000.0f);
	glUniform1f(uniforms.ssao.ssao_level, show_ao ? (show_shading ? 0.3f : 1.0f) : 0.0f);
	glUniform1i(uniforms.ssao.weight_by_angle, weight_by_angle ? 1 : 0);
	glUniform1i(uniforms.ssao.randomize_points, randomize_points ? 1 : 0);
	glUniform1ui(uniforms.ssao.point_count, point_count);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbo_textures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fbo_textures[1]);

	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(quad_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	float depth_min = FLT_MAX, depth_max = FLT_MIN;

	for (size_t i = 0; i < win_x; i++)
	{
		for (size_t j = 0; j < win_y; j++)
		{
			size_t depth_index = i * win_y + j;

			float val = depth_pixels[depth_index];

			if (val < depth_min)
				depth_min = val;

			if (val > depth_max)
				depth_max = val;
		}
	}

	for (size_t i = 0; i < win_x; i++)
	{
		for (size_t j = 0; j < win_y; j++)
		{
			size_t depth_index = i * win_y + j;

			float val = depth_pixels[depth_index];

			val = val - depth_min;
			val *= depth_max / (depth_max - depth_min);

			depth_pixels[depth_index] = val;
		}
	}

	vector<unsigned char> fbpixels(4 * static_cast<size_t>(win_x) * static_cast<size_t>(win_y));
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, win_x, win_y, GL_RGBA, GL_UNSIGNED_BYTE, &fbpixels[0]);

	vector<unsigned char> fbpixels_blurred = fbpixels;
	vector<unsigned char> target_pixels = fbpixels;

	blur_image(fbpixels_blurred, win_x, win_y, 4, 30);

	for (size_t i = 0; i < win_x; i++)
	{
		for (size_t j = 0; j < win_y; j++)
		{
			size_t depth_index = i * win_y + j;
			size_t fb_index = 4 * depth_index;
			
			float t = depth_pixels[depth_index];

			float r0 = fbpixels[fb_index];
			float g0 = fbpixels[fb_index + 1];
			float b0 = fbpixels[fb_index + 2];

			float r1 = fbpixels_blurred[fb_index];
			float g1 = fbpixels_blurred[fb_index + 1];
			float b1 = fbpixels_blurred[fb_index + 2];

			float r2 = (1.0f - t) * r0 + t * r1;
			float g2 = (1.0f - t) * g0 + t * g1;
			float b2 = (1.0f - t) * b0 + t * b1;

			//unsigned char val = static_cast<unsigned char>(depth_pixels[depth_index] * 255.0f);

			target_pixels[fb_index + 0] = static_cast<unsigned char>(r2);
			target_pixels[fb_index + 1] = static_cast<unsigned char>(g2);
			target_pixels[fb_index + 2] = static_cast<unsigned char>(b2);
			target_pixels[fb_index + 3] = 255;
		}
	}

	glDrawPixels(win_x, win_y, GL_RGBA, GL_UNSIGNED_BYTE, &target_pixels[0]);





	if (draw_console_checkbox->get_int_val() && log_system.get_contents_size() > 0)
	{
		size_t char_x_pos = 10;
		size_t char_y_pos = 30;

		RGB text_colour;
		text_colour.r = 255;
		text_colour.g = 255;
		text_colour.b = 255;

		vector<unsigned char> fbpixels(4 * static_cast<size_t>(win_x) * static_cast<size_t>(win_y));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, win_x, win_y, GL_RGBA, GL_UNSIGNED_BYTE, &fbpixels[0]);

		// Do anything you like here... for instance, use OpenCV for convolution

		for (size_t i = 0; i < log_system.get_contents_size(); i++)
		{
			string s;
			log_system.get_string_from_contents(i, s);
			print_sentence(fbpixels, win_x, win_y, char_x_pos, char_y_pos, s, text_colour);
			char_y_pos += 20;
		}

		glDrawPixels(win_x, win_y, GL_RGBA, GL_UNSIGNED_BYTE, &fbpixels[0]);
	}


	glutSwapBuffers();
}


void reshape_func(int width, int height)
{
	win_x = width;
	win_y = height;

	if (win_x < 1)
		win_x = 1;

	if (win_y < 1)
		win_y = 1;

	glutSetWindow(win_id);
	glutReshapeWindow(win_x, win_y);
	glViewport(0, 0, win_x, win_y);
}

void keyboard_func(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 's':
	{


		break;
	}

	default:
		break;
	}
}

void mouse_func(int button, int state, int x, int y)
{
	if (GLUT_LEFT_BUTTON == button)
	{
		if (GLUT_DOWN == state)
			lmb_down = true;
		else
			lmb_down = false;
	}
	else if (GLUT_MIDDLE_BUTTON == button)
	{
		if (GLUT_DOWN == state)
			mmb_down = true;
		else
			mmb_down = false;
	}
	else if (GLUT_RIGHT_BUTTON == button)
	{
		if (GLUT_DOWN == state)
			rmb_down = true;
		else
			rmb_down = false;
	}
}


void motion_func(int x, int y)
{
	int prev_mouse_x = mouse_x;
	int prev_mouse_y = mouse_y;

	mouse_x = x;
	mouse_y = y;

	int mouse_delta_x = mouse_x - prev_mouse_x;
	int mouse_delta_y = prev_mouse_y - mouse_y;

	if (true == lmb_down && (0 != mouse_delta_x || 0 != mouse_delta_y))
	{
		// Rotate camera
		main_camera.u -= static_cast<float>(mouse_delta_y) * u_spacer;
		main_camera.v += static_cast<float>(mouse_delta_x) * v_spacer;
	}
	else if (true == rmb_down && (0 != mouse_delta_y))
	{
		// Move camera
		main_camera.w -= static_cast<float>(mouse_delta_y) * w_spacer;

		if (main_camera.w < 1.1f)
			main_camera.w = 1.1f;
		else if (main_camera.w > 20.0f)
			main_camera.w = 20.0f;
	}
}



void setup_gui(void)
{
	glui = GLUI_Master.create_glui_subwindow(win_id, GLUI_SUBWINDOW_RIGHT);

	generate_mesh_button = glui->add_button(const_cast<char*>("Generate mesh"), 0, generate_cancel_button_func);

	glui->add_separator();

	equation_edittext = glui->add_edittext(const_cast<char*>("Equation:"), 0, const_cast<char*>("Z = sin(Z) + C*sin(Z)"), 3, control_cb);
	equation_edittext->set_text("Z = sin(Z) + C*sin(Z)");
	equation_edittext->set_w(150);

	glui->add_separator();

	draw_console_checkbox = glui->add_checkbox("Draw console text");
	draw_console_checkbox->set_int_val(1);
	draw_axis_checkbox = glui->add_checkbox("Draw axis");
	draw_axis_checkbox->set_int_val(1);

	randomize_c_checkbox = glui->add_checkbox("Randomize C");
	use_pedestal_checkbox = glui->add_checkbox("Use pedestal");
	use_pedestal_checkbox->set_int_val(1);

	pedestal_y_start_edittext = glui->add_edittext(const_cast<char*>("Pedestal y start:"), 0, const_cast<char*>("1.0"), 3, control_cb);
	pedestal_y_start_edittext->set_text("0.0");

	pedestal_y_end_edittext = glui->add_edittext(const_cast<char*>("Pedestal y end:"), 0, const_cast<char*>("1.0"), 3, control_cb);
	pedestal_y_end_edittext->set_text("0.15");


	glui->add_separator();

	obj_panel = glui->add_panel(const_cast<char*>("Constant"));

	obj_panel->set_alignment(GLUI_ALIGN_LEFT);

	c_x_edittext = glui->add_edittext_to_panel(obj_panel, const_cast<char*>("C.x:"), -1, const_cast<char*>("0.2866"), 3, control_cb);
	c_x_edittext->set_text("0.2866");

	c_y_edittext = glui->add_edittext_to_panel(obj_panel, const_cast<char*>("C.y:"), -1, const_cast<char*>("0.5133"), 3, control_cb);
	c_y_edittext->set_text("0.5133");

	c_z_edittext = glui->add_edittext_to_panel(obj_panel, const_cast<char*>("C.z:"), -1, const_cast<char*>("0.46"), 3, control_cb);
	c_z_edittext->set_text("0.46");

	c_w_edittext = glui->add_edittext_to_panel(obj_panel, const_cast<char*>("C.w:"), -1, const_cast<char*>("0.2467"), 3, control_cb);
	c_w_edittext->set_text("0.2467");

	obj_panel2 = glui->add_panel(const_cast<char*>("Various parameters"));

	obj_panel2->set_alignment(GLUI_ALIGN_LEFT);

	z_w_edittext = glui->add_edittext_to_panel(obj_panel2, const_cast<char*>("Z.w:"), -1, const_cast<char*>("0.0"), 3, control_cb);
	z_w_edittext->set_text("0.0");

	iterations_edittext = glui->add_edittext_to_panel(obj_panel2, const_cast<char*>("Max iterations:"), -1, const_cast<char*>("8"), 3, control_cb);
	iterations_edittext->set_text("8");

	resolution_edittext = glui->add_edittext_to_panel(obj_panel2, const_cast<char*>("Resolution:"), -1, const_cast<char*>("100"), 3, control_cb);
	resolution_edittext->set_text("100");

	infinity_edittext = glui->add_edittext_to_panel(obj_panel2, const_cast<char*>("Infinity:"), -1, const_cast<char*>("4.0"), 3, control_cb);
	infinity_edittext->set_text("4.0");

	obj_panel3 = glui->add_panel(const_cast<char*>("Space min/max"));

	obj_panel3->set_alignment(GLUI_ALIGN_LEFT);

	x_min_edittext = glui->add_edittext_to_panel(obj_panel3, const_cast<char*>("X min:"), -1, const_cast<char*>("-1.5"), 3, control_cb);
	y_min_edittext = glui->add_edittext_to_panel(obj_panel3, const_cast<char*>("Y min:"), -1, const_cast<char*>("-1.5"), 3, control_cb);
	z_min_edittext = glui->add_edittext_to_panel(obj_panel3, const_cast<char*>("Z min:"), -1, const_cast<char*>("-1.5"), 3, control_cb);
	x_min_edittext->set_text("-1.5");
	y_min_edittext->set_text("-1.5");
	z_min_edittext->set_text("-1.5");

	x_max_edittext = glui->add_edittext_to_panel(obj_panel3, const_cast<char*>("X max:"), -1, const_cast<char*>("1.5"), 3, control_cb);
	y_max_edittext = glui->add_edittext_to_panel(obj_panel3, const_cast<char*>("Y max:"), -1, const_cast<char*>("1.5"), 3, control_cb);
	z_max_edittext = glui->add_edittext_to_panel(obj_panel3, const_cast<char*>("Z max:"), -1, const_cast<char*>("1.5"), 3, control_cb);
	x_max_edittext->set_text("1.5");
	y_max_edittext->set_text("1.5");
	z_max_edittext->set_text("1.5");
}





#endif