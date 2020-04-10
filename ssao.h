#ifndef ssao_h
#define ssao_h



// This is the copyright notice that came with the SSAO code:
/*
 * Copyright © 2012-2015 Graham Sellers
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
#pragma comment(lib, "glui64bit")

#endif



#include <iostream>
using namespace std;

#include "vertex_fragment_shader.h"
#include "matrix_utils.h"
#include "uv_camera.h"
#include "mesh.h"
#include "glui.h"

#include <sstream>
using std::ostringstream;



float xy_aspect;
int   last_x, last_y;
GLUI* glui;// , * glui2;
GLUI_Checkbox* checkbox;
GLUI_Spinner* spinner, * light0_spinner, * light1_spinner;
GLUI_RadioGroup* radio;


/** Pointers to the windows and some of the controls we'll create **/
GLUI * glui2;


GLUI_Panel* obj_panel, * obj_panel2, * obj_panel3;

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


int   wireframe = 0;
int   obj = 0;
int   segments = 8;
char  text[200] = { "Hello World!" };

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

bool generate_button = true;


// Text drawing code originally from "GLUT Tutorial -- Bitmap Fonts and Orthogonal Projections" by A R Fernandes
void render_string(int x, const int y, void* font, const string& text)
{
	for (size_t i = 0; i < text.length(); i++)
	{
		glRasterPos2i(x, y);
		glutBitmapCharacter(font, text[i]);
		x += glutBitmapWidth(font, text[i]) + 1;
	}
}


void draw_text(void)
{
	// Text drawing code originally from "GLUT Tutorial -- Bitmap Fonts and Orthogonal Projections" by A R Fernandes
	// http://www.lighthouse3d.com/opengl/glut/index.php?bmpfontortho
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, win_x, 0, win_y);
	glScalef(1, -1, 1); // Neat. :)
	glTranslatef(0, -static_cast<float>(win_y), 0); // Neat. :)
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	vertex_3 background_colour(0.0f, 0.0f, 0.0f);
	vertex_3 control_list_colour(0.9f, 0.9f, 0.9f);

	glColor3f(control_list_colour.x, control_list_colour.y, control_list_colour.z);

	size_t break_size = 22;
	size_t start = 20;
	ostringstream oss;

	render_string(10, start, GLUT_BITMAP_HELVETICA_18, string("Mouse controls:"));
	render_string(10, start + 1 * break_size, GLUT_BITMAP_HELVETICA_18, string("  LMB + drag: Rotate camera"));
	render_string(10, start + 2 * break_size, GLUT_BITMAP_HELVETICA_18, string("  RMB + drag: Zoom camera"));

	render_string(10, start + 4 * break_size, GLUT_BITMAP_HELVETICA_18, string("Keyboard controls:"));
	render_string(10, start + 5 * break_size, GLUT_BITMAP_HELVETICA_18, string("  q: Draw mesh"));
	render_string(10, start + 6 * break_size, GLUT_BITMAP_HELVETICA_18, string("  w: Draw axis"));
	render_string(10, start + 7 * break_size, GLUT_BITMAP_HELVETICA_18, string("  e: Draw text"));

	render_string(10, start + 9 * break_size, GLUT_BITMAP_HELVETICA_18, string("  u: Rotate camera +u"));
	render_string(10, start + 10 * break_size, GLUT_BITMAP_HELVETICA_18, string("  i: Rotate camera -u"));
	render_string(10, start + 11 * break_size, GLUT_BITMAP_HELVETICA_18, string("  o: Rotate camera +v"));
	render_string(10, start + 12 * break_size, GLUT_BITMAP_HELVETICA_18, string("  p: Rotate camera -v"));



	vertex_3 eye = main_camera.eye;
	vertex_3 eye_norm = eye;
	eye_norm.normalize();

	oss.clear();
	oss.str("");
	oss << "Camera position: " << eye.x << ' ' << eye.y << ' ' << eye.z;
	render_string(10, win_y - 2 * break_size, GLUT_BITMAP_HELVETICA_18, oss.str());

	oss.clear();
	oss.str("");
	oss << "Camera position (normalized): " << eye_norm.x << ' ' << eye_norm.y << ' ' << eye_norm.z;
	render_string(10, win_y - break_size, GLUT_BITMAP_HELVETICA_18, oss.str());

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	// End text drawing code.
}



void generate_cancel_button_func(int control)
{
	//if (true == generate_button)
	//{
	//	button->set_name(const_cast<char*>("Generate"));
	//}
	//else
	//{
	//	button->set_name(const_cast<char*>("Cancel"));
	//}

	//button->enabled = false;

}

void export_button_func(int control)
{


}



void control_cb(int control)
{
	/********************************************************************
	  Here we'll print the user id of the control that generated the
	  callback, and we'll also explicitly get the values of each control.
	  Note that we really didn't have to explicitly get the values, since
	  they are already all contained within the live variables:
	  'wireframe',  'segments',  'obj',  and 'text'
	  ********************************************************************/

	//printf("callback: %d\n", control);
	//printf("                 text: %s\n", edittext->get_text());

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


	//int tx, ty, tw, th;
	//GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
	//glViewport(tx, ty, tw, th);

	//xy_aspect = (float)tw / (float)th;

	glutPostRedisplay();
}

void myGlutIdle(void)
{
	/* According to the GLUT specification, the current window is
	   undefined during an idle callback.  So we need to explicitly change
	   it if necessary */
	if (glutGetWindow() != win_id)
		glutSetWindow(win_id);

	glutPostRedisplay();
}


vector<triangle_index> triangle_indices;
vector<vertex_3_with_normal> vertices_with_face_normals;


vertex_fragment_shader render;
vertex_fragment_shader ssao;



GLuint fractal_vao;
GLuint      render_fbo;
GLuint      fbo_textures[3];
GLuint      quad_vao;
GLuint      points_buffer;


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

	*((unsigned int *) &res) = (tmp >> 9) | 0x3F800000;

	return (res - 1.0f);
}


void load_shaders()
{
	// Set up shader
	if(false == render.init("render.vs.glsl", "render.fs.glsl"))
	{
		cout << "Could not load render shader" << endl;
		return;
	}

	// Set up shader
	if(false == ssao.init("ssao.vs.glsl", "ssao.fs.glsl"))
	{
		cout << "Could not load SSAO shader" << endl;
		return;
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
}

bool init(void)
{
	ssao_level = 1.0f;
	ssao_radius = 0.05f;
	show_shading = true;
	show_ao = true;
	weight_by_angle = true;
	randomize_points = true;
	point_count = 10;

	// Transfer vertex data to GPU
	GLuint buffers[2];

	glGenVertexArrays(1, &fractal_vao);
	glBindVertexArray(fractal_vao);
    glGenBuffers(1, &buffers[0]);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices_with_face_normals.size()*6*sizeof(float), &vertices_with_face_normals[0], GL_STATIC_DRAW);
    
	// Set up vertex positions
	glVertexAttribPointer(0, 6/2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

	// Set up vertex normals
    glVertexAttribPointer(1, 6/2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(6/2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

	// Transfer index data to GPU
	glGenBuffers(1, &buffers[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_indices.size()*3*sizeof(GLuint), &triangle_indices[0], GL_STATIC_DRAW);




    load_shaders();

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

void display_func(void)
{
	static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
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

	glBindVertexArray(fractal_vao);
	glDrawElements(GL_TRIANGLES, triangle_indices.size()*3, GL_UNSIGNED_INT, 0);


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
	

	draw_text();


	glutSwapBuffers();
}

void idle_func(void)
{ 
	glutPostRedisplay();
}

void reshape_func(int width, int height)
{
	win_x = width;
	win_y = height;

	if(win_x < 1)
		win_x = 1;

	if(win_y < 1)
		win_y = 1;	

	glutSetWindow(win_id);
	glutReshapeWindow(win_x, win_y);
	glViewport(0, 0, win_x, win_y);
}

void keyboard_func(unsigned char key, int x, int y)
{
	switch(tolower(key))
	{
	case 'a':
		{
			break;
		}

	default:
		break;
	}
}

void mouse_func(int button, int state, int x, int y)
{
	if(GLUT_LEFT_BUTTON == button)
	{
		if(GLUT_DOWN == state)
			lmb_down = true;
		else
			lmb_down = false;
	}
	else if(GLUT_MIDDLE_BUTTON == button)
	{
		if(GLUT_DOWN == state)
			mmb_down = true;
		else
			mmb_down = false;
	}
	else if(GLUT_RIGHT_BUTTON == button)
	{
		if(GLUT_DOWN == state)
			rmb_down = true;
		else
			rmb_down = false;
	}
}

void myGlutMotion(int x, int y)
{
	glutPostRedisplay();
}


void motion_func(int x, int y)
{
	int prev_mouse_x = mouse_x;
	int prev_mouse_y = mouse_y;

	mouse_x = x;
	mouse_y = y;

	int mouse_delta_x = mouse_x - prev_mouse_x;
	int mouse_delta_y = prev_mouse_y - mouse_y;

	if(true == lmb_down && (0 != mouse_delta_x || 0 != mouse_delta_y))
	{
		// Rotate camera
		main_camera.u -= static_cast<float>(mouse_delta_y)*u_spacer;
		main_camera.v += static_cast<float>(mouse_delta_x)*v_spacer;
	}
	else if(true == rmb_down && (0 != mouse_delta_y))
	{
		// Move camera
		main_camera.w -= static_cast<float>(mouse_delta_y)*w_spacer;

		if(main_camera.w < 1.1f)
			main_camera.w = 1.1f;
		else if(main_camera.w > 20.0f)
			main_camera.w = 20.0f;
	}
}

void passive_motion_func(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
}

#endif