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
#include "GLUI/glui.h"
#include "string_utilities.h"
using namespace string_utilities;
#include "eqparse.h"
#include "marching_cubes.h"
using namespace marching_cubes;



class fractal_set_parameters
{
public:
	string equation_text;
	bool randomize_c;
	bool use_pedestal;
	float pedestal_y_start;
	float pedestal_y_end;
	float C_x, C_y, C_z, C_w;
	float Z_w;
	short unsigned int max_iterations;
	short unsigned int resolution;
	float infinity;
	float x_min, x_max;
	float y_min, y_max;
	float z_min, z_max;
};

vector<triangle> triangles;
vector<triangle_index> triangle_indices;
vector<vertex_3_with_normal> vertices_with_face_normals;


bool write_triangles_to_binary_stereo_lithography_file(atomic_bool &stop_flag, const vector<triangle>& triangles, const char* const file_name)
{
	cout << "Triangle count: " << triangles.size() << endl;

	if (0 == triangles.size())
		return false;

	// Write to file.
	ofstream out(file_name, ios_base::binary);

	if (out.fail())
		return false;

	const size_t header_size = 80;
	vector<char> buffer(header_size, 0);
	unsigned int num_triangles = static_cast<unsigned int>(triangles.size()); // Must be 4-byte unsigned int.
	vertex_3 normal;


	// Copy everything to a single buffer.
	// We do this here because calling ofstream::write() only once PER MESH is going to 
	// send the data to disk faster than if we were to instead call ofstream::write()
	// thirteen times PER TRIANGLE.
	// Of course, the trade-off is that we are using 2x the RAM than what's absolutely required,
	// but the trade-off is often very much worth it (especially so for meshes with millions of triangles).
	cout << "Generating normal/vertex/attribute buffer" << endl;

	// Enough bytes for twelve 4-byte floats plus one 2-byte integer, per triangle.
	const size_t data_size = (12 * sizeof(float) + sizeof(short unsigned int)) * num_triangles;
	buffer.resize(data_size, 0);

	// Use a pointer to assist with the copying.
	// Should probably use std::copy() instead, but memcpy() does the trick, so whatever...
	char* cp = &buffer[0];

	for (vector<triangle>::const_iterator i = triangles.begin(); i != triangles.end(); i++)
	{
		if (stop_flag)
			break;

		// Get face normal.
		vertex_3 v0 = i->vertex[1] - i->vertex[0];
		vertex_3 v1 = i->vertex[2] - i->vertex[0];
		normal = v0.cross(v1);
		normal.normalize();

		memcpy(cp, &normal.x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &normal.y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &normal.z, sizeof(float)); cp += sizeof(float);

		memcpy(cp, &i->vertex[0].x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[0].y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[0].z, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[1].x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[1].y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[1].z, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[2].x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[2].y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[2].z, sizeof(float)); cp += sizeof(float);

		cp += sizeof(short unsigned int);
	}

	// Write blank header.
	out.write(reinterpret_cast<const char*>(&(buffer[0])), header_size);

	if (stop_flag)
		num_triangles = 0;

	// Write number of triangles.
	out.write(reinterpret_cast<const char*>(&num_triangles), sizeof(unsigned int));


	cout << "Writing " << data_size / 1048576 << " MB of data to binary Stereo Lithography file: " << file_name << endl;

	if(false == stop_flag)
		out.write(reinterpret_cast<const char*>(&buffer[0]), data_size);
	
	cout << "Done writing out.stl" << endl;

	out.close();

	return true;
}




void thread_func(atomic_bool& stop_flag, atomic_bool& thread_is_running_flag, fractal_set_parameters p, vector<triangle>& t, vector<string>& vs, mutex& m)
{
	thread_is_running_flag = true;

	m.lock();
	t.clear();
	m.unlock();

	cout << "starting Thread_func" << endl;

	bool make_border = true;

	quaternion C;
	C.x = p.C_x;
	C.y = p.C_y;
	C.z = p.C_z;
	C.w = p.C_w;

	string error_string;
	quaternion_julia_set_equation_parser eqparser;

	if (false == eqparser.setup(p.equation_text, error_string, C))
	{
		cout << "Equation error: " << error_string << endl;
		return;
	}
	else
	{
		cout << "Equation " << p.equation_text << " compiled successfully" << endl;
	}


	// When adding a border, use a value that is "much" greater than the threshold.
	const float border_value = 1.0f + p.infinity;

	vector<float> xyplane0(p.resolution * p.resolution, 0);
	vector<float> xyplane1(p.resolution * p.resolution, 0);

	const float step_size_x = (p.x_max - p.x_min) / (p.resolution - 1);
	const float step_size_y = (p.y_max - p.y_min) / (p.resolution - 1);
	const float step_size_z = (p.z_max - p.z_min) / (p.resolution - 1);

	size_t z = 0;

	quaternion Z(p.x_min, p.y_min, p.z_min, p.Z_w);

	// Calculate 0th xy plane.
	for (size_t x = 0; x < p.resolution; x++, Z.x += step_size_x)
	{
		Z.y = p.y_min;

		for (size_t y = 0; y < p.resolution; y++, Z.y += step_size_y)
		{
			if (stop_flag)
			{
				thread_is_running_flag = false;
				return;
			}

			if (true == make_border && (x == 0 || y == 0 || z == 0 || x == p.resolution - 1 || y == p.resolution - 1 || z == p.resolution - 1))
				xyplane0[x * p.resolution + y] = border_value;
			else
				xyplane0[x * p.resolution + y] = eqparser.iterate(Z, p.max_iterations, p.infinity);
		}
	}

	// Prepare for 1st xy plane.
	z++;
	Z.z += step_size_z;



	size_t box_count = 0;


	// Calculate 1st and subsequent xy planes.
	for (; z < p.resolution; z++, Z.z += step_size_z)
	{
		Z.x = p.z_min;

		cout << "Calculating triangles from xy-plane pair " << z << " of " << p.resolution - 1 << endl;

		for (size_t x = 0; x < p.resolution; x++, Z.x += step_size_x)
		{
			Z.y = p.y_min;

			for (size_t y = 0; y < p.resolution; y++, Z.y += step_size_y)
			{
				if (stop_flag)
				{
					thread_is_running_flag = false;
					return;
				}

				if (true == make_border && (x == 0 || y == 0 || z == 0 || x == p.resolution - 1 || y == p.resolution - 1 || z == p.resolution - 1))
					xyplane1[x * p.resolution + y] = border_value;
				else
					xyplane1[x * p.resolution + y] = eqparser.iterate(Z, p.max_iterations, p.infinity);
			}
		}

		// Calculate triangles for the xy-planes corresponding to z - 1 and z by marching cubes.
		tesselate_adjacent_xy_plane_pair(stop_flag,
			box_count,
			xyplane0, xyplane1,
			z - 1,
			t,
			p.infinity, // Use threshold as isovalue.
			p.x_min, p.x_max, p.resolution,
			p.y_min, p.y_max, p.resolution,
			p.z_min, p.z_max, p.resolution);


		if (stop_flag)
		{
			thread_is_running_flag = false;
			return;
		}

		// Swap memory pointers (fast) instead of performing a memory copy (slow).
		xyplane1.swap(xyplane0);
	}

	cout << endl;
	
	m.lock();
	get_triangle_indices_and_vertices_with_face_normals_from_triangles(stop_flag, m, t, triangle_indices, vertices_with_face_normals);
	m.unlock();

	write_triangles_to_binary_stereo_lithography_file(stop_flag, t, "out.stl");
	

	thread_is_running_flag = false;
	return;
}

GLuint fractal_buffers[2];
GLuint fractal_vao = 0;
GLuint      render_fbo = 0;
GLuint      fbo_textures[3] = { 0, 0, 0 };
GLuint      quad_vao = 0;
GLuint      points_buffer = 0;
bool gpu_holds_data = false;

thread* gen_thread = 0;
atomic_bool stop = false;
atomic_bool thread_is_running = false;
atomic_bool uploaded_to_gpu = false;
vector<string> string_log;
mutex thread_mutex;




GLUI* glui, * glui2;

GLUI_Panel* obj_panel, * obj_panel2, * obj_panel3;

GLUI_Button* generate_mesh_button, * export_to_stl_button;

GLUI_Checkbox* randomize_c_checkbox, * use_pedestal_checkbox;

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





class BMP
{
public:
	std::uint32_t width, height;
	std::uint16_t BitsPerPixel;
	std::vector<unsigned char> Pixels;

public:
	BMP(void)
	{

	}

	bool load(const char* filePath);
	std::vector<std::uint8_t> GetPixels() const { return this->Pixels; }
	std::uint32_t GetWidth() const { return this->width; }
	std::uint32_t GetHeight() const { return this->height; }
	bool HasAlphaChannel() { return BitsPerPixel == 32; }
};



bool BMP::load(const char* FilePath)
{
	std::fstream hFile(FilePath, std::ios::in | std::ios::binary);
	if (!hFile.is_open()) return false;

	hFile.seekg(0, std::ios::end);
	std::size_t Length = hFile.tellg();
	hFile.seekg(0, std::ios::beg);
	std::vector<std::uint8_t> FileInfo(Length);
	hFile.read(reinterpret_cast<char*>(FileInfo.data()), 54);

	if (FileInfo[0] != 'B' && FileInfo[1] != 'M')
	{
		hFile.close();
		return false;
	}

	// if (FileInfo[28] != 24 && FileInfo[28] != 32)
	if (FileInfo[28] != 32)
	{
		hFile.close();
		return false;
	}

	BitsPerPixel = FileInfo[28];
	width = FileInfo[18] + (FileInfo[19] << 8);
	height = FileInfo[22] + (FileInfo[23] << 8);
	std::uint32_t PixelsOffset = FileInfo[10] + (FileInfo[11] << 8);
	std::uint32_t size = ((width * BitsPerPixel + 31) / 32) * 4 * height;
	Pixels.resize(size);

	hFile.seekg(PixelsOffset, std::ios::beg);
	hFile.read(reinterpret_cast<char*>(Pixels.data()), size);
	hFile.close();


	// Reverse row order
	short unsigned int num_rows_to_swap = height;
	vector<unsigned char> buffer(static_cast<size_t>(width) * 4);

	if (0 != height % 2)
		num_rows_to_swap--;

	num_rows_to_swap /= 2;

	for (size_t i = 0; i < num_rows_to_swap; i++)
	{
		size_t y_first = i * static_cast<size_t>(width) * 4;
		size_t y_last = (static_cast<size_t>(height) - 1 - i) * static_cast<size_t>(width) * 4;

		memcpy(&buffer[0], &Pixels[y_first], static_cast<size_t>(width) * 4);
		memcpy(&Pixels[y_first], &Pixels[y_last], static_cast<size_t>(width) * 4);
		memcpy(&Pixels[y_last], &buffer[0], static_cast<size_t>(width) * 4);
	}

	return true;
}



void generate_cancel_button_func(int control)
{
	if (generate_button == false)
	{
		cout << "user clicked cancel" << endl;
		stop = true;
		uploaded_to_gpu = false;

		cout << "killing thread" << endl;

		if (gen_thread != 0)
		{
			stop = true;

			gen_thread->join();

			delete gen_thread;
			gen_thread = 0;
			stop = true;
		//	thread_is_running = false;
		}

		cout << "done killing thread" << endl;

		generate_button = true;
		generate_mesh_button->set_name(const_cast<char*>("Generate mesh"));

	}
	else
	{
		cout << "user clicked generate mesh" << endl;

		fractal_set_parameters p;

		p.equation_text = equation_edittext->text;

		if (p.equation_text == "")
		{
			cout << "blank equation text" << endl;
			return;
		}

		p.randomize_c = randomize_c_checkbox->get_int_val();
		p.use_pedestal = use_pedestal_checkbox->get_int_val();

		string temp_string;

		temp_string = pedestal_y_start_edittext->text;

		if (false == is_real_number(temp_string))
		{
			cout << "pedestal y start is not a real number" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.pedestal_y_start;
			cout << p.pedestal_y_start << endl;
		}

		temp_string = pedestal_y_end_edittext->text;

		if (false == is_real_number(temp_string))
		{
			cout << "pedestal y end is not a real number" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.pedestal_y_end;
		}

		if (p.pedestal_y_start < 0 || p.pedestal_y_start > 1)
		{
			cout << "pedestal y start must be between 0 and 1" << endl;
			return;
		}

		if (p.pedestal_y_end < 0 || p.pedestal_y_end > 1)
		{
			cout << "pedestal y end must be between 0 and 1" << endl;
			return;
		}

		if (p.pedestal_y_start >= p.pedestal_y_end)
		{
			cout << "Y start must be smaller than y_end" << endl;
			return;
		}




		temp_string = c_x_edittext->text;

		if (false == is_real_number(temp_string))
		{
			cout << "c.x  is not a real number" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.C_x;
		}

		temp_string = c_y_edittext->text;

		if (false == is_real_number(temp_string))
		{
			cout << "c.y  is not a real number" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.C_y;
		}

		temp_string = c_z_edittext->text;

		if (false == is_real_number(temp_string))
		{
			cout << "c.z  is not a real number" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.C_z;
		}

		temp_string = c_w_edittext->text;

		if (false == is_real_number(temp_string))
		{
			cout << "c.w  is not a real number" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.C_w;
		}

		temp_string = x_min_edittext->text;

		if (false == is_real_number(temp_string))
		{
			cout << "x min  is not a real number" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.x_min;
		}

		temp_string = y_min_edittext->text;

		if (false == is_real_number(temp_string))
		{
			cout << "y min  is not a real number" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.y_min;
		}

		temp_string = z_min_edittext->text;

		if (false == is_real_number(temp_string))
		{
			cout << "z min  is not a real number" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.z_min;
		}




		temp_string = x_max_edittext->text;

		if (false == is_real_number(temp_string))
		{
			cout << "x max  is not a real number" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.x_max;
		}

		temp_string = y_max_edittext->text;

		if (false == is_real_number(temp_string))
		{
			cout << "y max  is not a real number" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.y_max;
		}

		temp_string = z_max_edittext->text;

		if (false == is_real_number(temp_string))
		{
			cout << "z max  is not a real number" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.z_max;
		}

		if (p.x_min >= p.x_max)
		{
			cout << "x min must be less than x max" << endl;
			return;
		}

		if (p.y_min >= p.y_max)
		{
			cout << "y min must be less than y max" << endl;
			return;
		}

		if (p.z_min >= p.z_max)
		{
			cout << "z min must be less than z max" << endl;
			return;
		}

		temp_string = z_w_edittext->text;

		if (false == is_real_number(temp_string))
		{
			cout << "z.w  is not a real number" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.Z_w;
		}


		temp_string = infinity_edittext->text;

		if (false == is_real_number(temp_string))
		{
			cout << "infinity  is not a real number" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.infinity;
		}


		temp_string = iterations_edittext->text;

		if (false == is_unsigned_short_int(temp_string))
		{
			cout << "max iterations is not a short unsigned int" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.max_iterations;
		}

		temp_string = resolution_edittext->text;

		if (false == is_unsigned_short_int(temp_string))
		{
			cout << "resolution is not a short unsigned int" << endl;
			return;
		}
		else
		{
			istringstream iss(temp_string);
			iss >> p.resolution;

			cout << "Read res " << p.resolution << endl;
		}


		stop = false;
		thread_is_running = true;
		uploaded_to_gpu = false;

		if (gen_thread != 0)
		{
			cout << "killing existing thread" << endl;

			stop = true;
			gen_thread->join();

			delete gen_thread;
			gen_thread = 0;
			stop = false;
		}

		cout << "Starting new thread" << endl;
\
		gen_thread = new thread(thread_func, ref(stop), ref(thread_is_running), p, ref(triangles), ref(string_log), ref(thread_mutex));

		generate_button = false;
		generate_mesh_button->set_name(const_cast<char*>("Cancel"));
	}
}


void control_cb(int control)
{
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
	if (false == thread_is_running && false == generate_button)
	{
		cout << "Thread completed" << endl;
		cout << "tris " << triangles.size() << endl;

		if (false == uploaded_to_gpu && false == stop && triangles.size() > 0)
		{
			cout << "uploading to gpu" << endl;

			thread_mutex.lock();

			if (glIsVertexArray(fractal_vao))
			{
				cout << "cleaning up fractal vao" << endl;
				glDeleteVertexArrays(1, &fractal_vao);
			}

			if (glIsBuffer(fractal_buffers[0]))
			{
				cout << "cleaning up buffer 0" << endl;
				glDeleteBuffers(1, &fractal_buffers[0]);
			}

			if (glIsBuffer(fractal_buffers[1]))
			{
				cout << "cleaning up buffer 1" << endl;
				glDeleteBuffers(1, &fractal_buffers[1]);
			}

			cout << "uploading vertex with normals" << endl;

			glGenVertexArrays(1, &fractal_vao);
			glBindVertexArray(fractal_vao);
			glGenBuffers(1, &fractal_buffers[0]);
			glBindBuffer(GL_ARRAY_BUFFER, fractal_buffers[0]);
			glBufferData(GL_ARRAY_BUFFER, vertices_with_face_normals.size() * 6 * sizeof(float), &vertices_with_face_normals[0], GL_STATIC_DRAW);

			// Set up vertex positions
			glVertexAttribPointer(0, 6 / 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			// Set up vertex normals
			glVertexAttribPointer(1, 6 / 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(6 / 2 * sizeof(GLfloat)));
			glEnableVertexAttribArray(1);

			cout << "uploading index data" << endl;
			// Transfer index data to GPU
			glGenBuffers(1, &fractal_buffers[1]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fractal_buffers[1]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_indices.size() * 3 * sizeof(GLuint), &triangle_indices[0], GL_STATIC_DRAW);
			thread_mutex.unlock();

			cout << "Done uploading to GPU" << endl;
			uploaded_to_gpu = true;
		}

		generate_button = true;
		generate_mesh_button->set_name(const_cast<char*>("Generate mesh"));
	}

	glutPostRedisplay();
}




vertex_fragment_shader render;
vertex_fragment_shader ssao;





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

	*((unsigned int*)&res) = (tmp >> 9) | 0x3F800000;

	return (res - 1.0f);
}


void load_shaders()
{
	// Set up shader
	if (false == render.init("render.vs.glsl", "render.fs.glsl"))
	{
		cout << "Could not load render shader" << endl;
		return;
	}

	// Set up shader
	if (false == ssao.init("ssao.vs.glsl", "ssao.fs.glsl"))
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


class RGB
{
public:
	unsigned char r, g, b;
};


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
	BMP font;

	if (false == font.load("font.bmp"))
	{
		cout << "could not load font.bmp" << endl;
		return false;
	}


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

	// Transfer vertex data to GPU

	if (false == uploaded_to_gpu)
	{
		if (glIsVertexArray(fractal_vao))
		{
			cout << "cleaning up fractal vao" << endl;
			glDeleteVertexArrays(1, &fractal_vao);
		}

		if (glIsBuffer(fractal_buffers[0]))
		{
			cout << "cleaning up buffer 0" << endl;
			glDeleteBuffers(1, &fractal_buffers[0]);
		}

		if (glIsBuffer(fractal_buffers[1]))
		{
			cout << "cleaning up buffer 1" << endl;
			glDeleteBuffers(1, &fractal_buffers[1]);
		}

		cout << "uploading vertex with normals" << endl;
		glGenVertexArrays(1, &fractal_vao);
		glBindVertexArray(fractal_vao);
		glGenBuffers(1, &fractal_buffers[0]);
		glBindBuffer(GL_ARRAY_BUFFER, fractal_buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, vertices_with_face_normals.size() * 6 * sizeof(float), &vertices_with_face_normals[0], GL_STATIC_DRAW);

		// Set up vertex positions
		glVertexAttribPointer(0, 6 / 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		// Set up vertex normals
		glVertexAttribPointer(1, 6 / 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(6 / 2 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		cout << "uploading index data" << endl;
		// Transfer index data to GPU
		glGenBuffers(1, &fractal_buffers[1]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fractal_buffers[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_indices.size() * 3 * sizeof(GLuint), &triangle_indices[0], GL_STATIC_DRAW);

		uploaded_to_gpu = true;
		cout << "Done uploading to GPU" << endl;
	}



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
	
	if (uploaded_to_gpu)
	{
		glBindVertexArray(fractal_vao);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(triangle_indices.size() * 3), GL_UNSIGNED_INT, 0);
	}

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




	size_t char_x_pos = 10;
	size_t char_y_pos = 30;

	RGB text_colour;
	text_colour.r = 255;
	text_colour.g = 255;
	text_colour.b = 255;

	vector<unsigned char> fbpixels(4 * static_cast<size_t>(win_x) * static_cast<size_t>(win_y));

	glReadPixels(0, 0, win_x, win_y, GL_RGBA, GL_UNSIGNED_BYTE, &fbpixels[0]);

	print_sentence(fbpixels, win_x, win_y, char_x_pos, char_y_pos, "Hello World1", text_colour);
	print_sentence(fbpixels, win_x, win_y, char_x_pos, char_y_pos + 20, "Hello World2", text_colour);

	glDrawPixels(win_x, win_y, GL_RGBA, GL_UNSIGNED_BYTE, &fbpixels[0]);

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

void myGlutMotion(int x, int y)
{
//	glutPostRedisplay();
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

void passive_motion_func(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
}

#endif