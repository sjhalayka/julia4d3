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

#include "logging_system.h"
#include "bmp.h"
#include "fractal_set_parameters.h"


logging_system log_system;

vector<GLfloat> vertex_data;




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

GLUI_Checkbox* rainbow_colouring_checkbox, * randomize_c_checkbox, * use_pedestal_checkbox;

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

vector<triangle> triangles;
vector<vertex_3_with_normal> vertices_with_face_normals;

GLuint fractal_buffers[2] = { 0, 0 };
GLuint fractal_vao = 0;
GLuint      render_fbo = 0;
GLuint      fbo_textures[3] = { 0, 0, 0 };
GLuint      quad_vao = 0;
GLuint      points_buffer = 0;
bool gpu_holds_data = false;

thread* gen_thread = 0;
atomic_bool stop = false;
atomic_bool thread_is_running = false;
atomic_bool vertex_data_refreshed = false;
vector<string> string_log;
mutex thread_mutex;

bool generate_button = true;
unsigned int triangle_buffer = 0;

class RGB
{
public:
	unsigned char r, g, b;
};


bool write_triangles_to_binary_stereo_lithography_file(const char* const file_name)
{
	ostringstream oss;

	oss.clear();
	oss.str("");
	oss << "Triangle count: " << triangles.size();
	thread_mutex.lock();
	log_system.add_string_to_contents(oss.str());
	thread_mutex.unlock();

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

	oss.clear();
	oss.str("");
	oss << "Generating normal/vertex/attribute buffer";
	thread_mutex.lock();
	log_system.add_string_to_contents(oss.str());
	thread_mutex.unlock();

	// Enough bytes for twelve 4-byte floats plus one 2-byte integer, per triangle.
	const size_t data_size = (12 * sizeof(float) + sizeof(short unsigned int)) * num_triangles;
	buffer.resize(data_size, 0);

	// Use a pointer to assist with the copying.
	// Should probably use std::copy() instead, but memcpy() does the trick, so whatever...
	char* cp = &buffer[0];

	for (vector<triangle>::const_iterator i = triangles.begin(); i != triangles.end(); i++)
	{
		if (stop)
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

	if (stop)
		num_triangles = 0;

	// Write number of triangles.
	out.write(reinterpret_cast<const char*>(&num_triangles), sizeof(unsigned int));

	oss.clear();
	oss.str("");
	oss << "Writing " << data_size / 1048576.0 << " MB of data to STL file: " << file_name;
	thread_mutex.lock();
	log_system.add_string_to_contents(oss.str());
	thread_mutex.unlock();

	if(false == stop)
		out.write(reinterpret_cast<const char*>(&buffer[0]), data_size);

	oss.clear();
	oss.str("");
	oss << "Done writing out.stl";
	thread_mutex.lock();
	log_system.add_string_to_contents(oss.str());
	thread_mutex.unlock();

	out.close();

	return true;
}



void get_vertices_with_face_normals_from_triangles(void)
{
	vector<vertex_3_with_index> v;

	vertices_with_face_normals.clear();

	if (0 == triangles.size())
		return;

	ostringstream oss;

	oss.clear();
	oss.str("");
	oss << "Welding vertices";
	thread_mutex.lock();
	log_system.add_string_to_contents(oss.str());
	thread_mutex.unlock();

	// Insert unique vertices into set.
	set<vertex_3_with_index> vertex_set;

	for (vector<triangle>::const_iterator i = triangles.begin(); i != triangles.end(); i++)
	{
		if (stop)
			return;

		vertex_set.insert(i->vertex[0]);
		vertex_set.insert(i->vertex[1]);
		vertex_set.insert(i->vertex[2]);
	}

	oss.clear();
	oss.str("");
	oss << "Vertices: " << vertex_set.size();
	thread_mutex.lock();
	log_system.add_string_to_contents(oss.str());
	thread_mutex.unlock();


	oss.clear();
	oss.str("");
	oss << "Generating vertex indices";
	thread_mutex.lock();
	log_system.add_string_to_contents(oss.str());
	thread_mutex.unlock();


	// Add indices to the vertices.
	for (set<vertex_3_with_index>::const_iterator i = vertex_set.begin(); i != vertex_set.end(); i++)
	{
		if (stop)
			return;

		size_t index = v.size();
		v.push_back(*i);
		v[index].index = static_cast<GLuint>(index);
	}

	vertex_set.clear();

	// Re-insert modified vertices into set.
	for (vector<vertex_3_with_index>::const_iterator i = v.begin(); i != v.end(); i++)
	{
		if (stop)
			return;

		vertex_set.insert(*i);
	}

	oss.clear();
	oss.str("");
	oss << "Assigning vertex indices to triangles";
	thread_mutex.lock();
	log_system.add_string_to_contents(oss.str());
	thread_mutex.unlock();


	// Find the three vertices for each triangle, by index.
	set<vertex_3_with_index>::iterator find_iter;

	for (vector<triangle>::iterator i = triangles.begin(); i != triangles.end(); i++)
	{
		if (stop)
			return;

		find_iter = vertex_set.find(i->vertex[0]);
		i->vertex[0].index = find_iter->index;

		find_iter = vertex_set.find(i->vertex[1]);
		i->vertex[1].index = find_iter->index;

		find_iter = vertex_set.find(i->vertex[2]);
		i->vertex[2].index = find_iter->index;
	}

	vertex_set.clear();

	oss.clear();
	oss.str("");
	oss << "Calculating normals";
	thread_mutex.lock();
	log_system.add_string_to_contents(oss.str());
	thread_mutex.unlock();

	vertices_with_face_normals.resize(v.size());

	// Assign per-triangle face normals
	for (size_t i = 0; i < triangles.size(); i++)
	{
		if (stop)
			return;

		vertex_3 v0 = triangles[i].vertex[1] - triangles[i].vertex[0];
		vertex_3 v1 = triangles[i].vertex[2] - triangles[i].vertex[0];
		vertex_3 fn = v0.cross(v1);
		fn.normalize();

		vertices_with_face_normals[triangles[i].vertex[0].index].nx += fn.x;
		vertices_with_face_normals[triangles[i].vertex[0].index].ny += fn.y;
		vertices_with_face_normals[triangles[i].vertex[0].index].nz += fn.z;
		vertices_with_face_normals[triangles[i].vertex[1].index].nx += fn.x;
		vertices_with_face_normals[triangles[i].vertex[1].index].ny += fn.y;
		vertices_with_face_normals[triangles[i].vertex[1].index].nz += fn.z;
		vertices_with_face_normals[triangles[i].vertex[2].index].nx += fn.x;
		vertices_with_face_normals[triangles[i].vertex[2].index].ny += fn.y;
		vertices_with_face_normals[triangles[i].vertex[2].index].nz += fn.z;
	}

	oss.clear();
	oss.str("");
	oss << "Generating final index/vertex data";
	thread_mutex.lock();
	log_system.add_string_to_contents(oss.str());
	thread_mutex.unlock();

	for (size_t i = 0; i < v.size(); i++)
	{
		if (stop)
			return;

		// Assign vertex spatial comoponents
		vertices_with_face_normals[i].x = v[i].x;
		vertices_with_face_normals[i].y = v[i].y;
		vertices_with_face_normals[i].z = v[i].z;

		// Normalize face normal
		vertex_3 temp_face_normal(vertices_with_face_normals[i].nx, vertices_with_face_normals[i].ny, vertices_with_face_normals[i].nz);
		temp_face_normal.normalize();

		vertices_with_face_normals[i].nx = temp_face_normal.x;
		vertices_with_face_normals[i].ny = temp_face_normal.y;
		vertices_with_face_normals[i].nz = temp_face_normal.z;
	}

	oss.clear();
	oss.str("");
	oss << "Done";
	thread_mutex.lock();
	log_system.add_string_to_contents(oss.str());
	thread_mutex.unlock();
}

void thread_func(fractal_set_parameters p)
{
	thread_is_running = true;

	triangles.clear();
	vertices_with_face_normals.clear();

	bool make_border = true;

	quaternion C;
	C.x = p.C_x;
	C.y = p.C_y;
	C.z = p.C_z;
	C.w = p.C_w;

	ostringstream oss;

	string error_string;
	quaternion_julia_set_equation_parser eqparser;

	if (false == eqparser.setup(p.equation_text, error_string, C))
	{
		oss.clear();
		oss.str("");
		oss << "Equation error: " << error_string;
		thread_mutex.lock();
		log_system.add_string_to_contents(oss.str());
		thread_mutex.unlock();

		return;
	}

	// When adding a border, use a value that is "much" greater than the threshold.
	const float border_value = 1.0f + p.infinity;

	size_t num_voxels = p.resolution * p.resolution;
	vector<float> xyplane0(num_voxels, 0);
	vector<float> xyplane1(num_voxels, 0);

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
			if (stop)
			{
				thread_is_running = false;
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

		oss.clear();
		oss.str("");
		oss << "Calculating triangles from xy-plane pair " << z << " of " << p.resolution - 1;
		thread_mutex.lock();
		log_system.add_string_to_contents(oss.str());
		thread_mutex.unlock();

		for (size_t x = 0; x < p.resolution; x++, Z.x += step_size_x)
		{
			Z.y = p.y_min;

			for (size_t y = 0; y < p.resolution; y++, Z.y += step_size_y)
			{
				if (stop)
				{
					thread_is_running = false;
					return;
				}

				if (true == make_border && (x == 0 || y == 0 || z == 0 || x == p.resolution - 1 || y == p.resolution - 1 || z == p.resolution - 1))
					xyplane1[x * p.resolution + y] = border_value;
				else
					xyplane1[x * p.resolution + y] = eqparser.iterate(Z, p.max_iterations, p.infinity);
			}
		}

		// Calculate triangles for the xy-planes corresponding to z - 1 and z by marching cubes.
		tesselate_adjacent_xy_plane_pair(stop,
			box_count,
			xyplane0, xyplane1,
			z - 1,
			triangles,
			p.infinity, // Use threshold as isovalue.
			p.x_min, p.x_max, p.resolution,
			p.y_min, p.y_max, p.resolution,
			p.z_min, p.z_max, p.resolution);


		if (stop)
		{
			thread_is_running = false;
			return;
		}

		// Swap memory pointers (fast) instead of performing a memory copy (slow).
		xyplane1.swap(xyplane0);
	}

	if (false == stop)
	{
		get_vertices_with_face_normals_from_triangles();
		write_triangles_to_binary_stereo_lithography_file("out.stl");
	}

	thread_is_running = false;
	return;
}











bool obtain_control_contents(fractal_set_parameters &p)
{
	p.equation_text = equation_edittext->text;

	if (p.equation_text == "")
	{
		cout << "blank equation text" << endl;
		return false;
	}

	p.randomize_c = randomize_c_checkbox->get_int_val();
	p.use_pedestal = use_pedestal_checkbox->get_int_val();

	string temp_string;

	temp_string = pedestal_y_start_edittext->text;

	if (false == is_real_number(temp_string))
	{
		cout << "pedestal y start is not a real number" << endl;
		return false;
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
		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.pedestal_y_end;
	}

	if (p.pedestal_y_start < 0 || p.pedestal_y_start > 1)
	{
		cout << "pedestal y start must be between 0 and 1" << endl;
		return false;
	}

	if (p.pedestal_y_end < 0 || p.pedestal_y_end > 1)
	{
		cout << "pedestal y end must be between 0 and 1" << endl;
		return false;
	}

	if (p.pedestal_y_start >= p.pedestal_y_end)
	{
		cout << "Y start must be smaller than y_end" << endl;
		return false;
	}




	temp_string = c_x_edittext->text;

	if (false == is_real_number(temp_string))
	{
		cout << "c.x  is not a real number" << endl;
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
		cout << "c.y  is not a real number" << endl;
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
		cout << "c.z  is not a real number" << endl;
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
		cout << "c.w  is not a real number" << endl;
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
		cout << "x min  is not a real number" << endl;
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
		cout << "y min  is not a real number" << endl;
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
		cout << "z min  is not a real number" << endl;
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
		cout << "x max  is not a real number" << endl;
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
		cout << "y max  is not a real number" << endl;
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
		cout << "z max  is not a real number" << endl;
		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.z_max;
	}

	if (p.x_min >= p.x_max)
	{
		cout << "x min must be less than x max" << endl;
		return false;
	}

	if (p.y_min >= p.y_max)
	{
		cout << "y min must be less than y max" << endl;
		return false;
	}

	if (p.z_min >= p.z_max)
	{
		cout << "z min must be less than z max" << endl;
		return false;
	}

	temp_string = z_w_edittext->text;

	if (false == is_real_number(temp_string))
	{
		cout << "z.w  is not a real number" << endl;
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
		cout << "infinity  is not a real number" << endl;
		return false;
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
		return false;
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
		return false;
	}
	else
	{
		istringstream iss(temp_string);
		iss >> p.resolution;

		cout << "Read res " << p.resolution << endl;
	}

	return true;
}

void generate_cancel_button_func(int control)
{
	if (generate_button == false)
	{
		stop = true;
		vertex_data_refreshed = false;

		if (gen_thread != 0)
		{
			stop = true;

			gen_thread->join();

			delete gen_thread;
			gen_thread = 0;
			stop = true;
		}

		generate_button = true;
		generate_mesh_button->set_name(const_cast<char*>("Generate mesh"));
	}
	else
	{
		fractal_set_parameters p;

		if (false == obtain_control_contents(p))
			return;

		stop = false;
		vertex_data_refreshed = false;

		if (gen_thread != 0)
		{
			stop = true;
			gen_thread->join();

			delete gen_thread;
			gen_thread = 0;
			stop = false;
		}

		gen_thread = new thread(thread_func, p);

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

	glutPostRedisplay();
}


RGB HSBtoRGB(unsigned short int hue_degree, unsigned char sat_percent, unsigned char bri_percent)
{
	float R = 0.0f;
	float G = 0.0f;
	float B = 0.0f;

	if (hue_degree > 359)
		hue_degree = 359;

	if (sat_percent > 100)
		sat_percent = 100;

	if (bri_percent > 100)
		bri_percent = 100;

	float hue_pos = 6.0f - ((static_cast<float>(hue_degree) / 359.0f) * 6.0f);

	if (hue_pos >= 0.0f && hue_pos < 1.0f)
	{
		R = 255.0f;
		G = 0.0f;
		B = 255.0f * hue_pos;
	}
	else if (hue_pos >= 1.0f && hue_pos < 2.0f)
	{
		hue_pos -= 1.0f;

		R = 255.0f - (255.0f * hue_pos);
		G = 0.0f;
		B = 255.0f;
	}
	else if (hue_pos >= 2.0f && hue_pos < 3.0f)
	{
		hue_pos -= 2.0f;

		R = 0.0f;
		G = 255.0f * hue_pos;
		B = 255.0f;
	}
	else if (hue_pos >= 3.0f && hue_pos < 4.0f)
	{
		hue_pos -= 3.0f;

		R = 0.0f;
		G = 255.0f;
		B = 255.0f - (255.0f * hue_pos);
	}
	else if (hue_pos >= 4.0f && hue_pos < 5.0f)
	{
		hue_pos -= 4.0f;

		R = 255.0f * hue_pos;
		G = 255.0f;
		B = 0.0f;
	}
	else
	{
		hue_pos -= 5.0f;

		R = 255.0f;
		G = 255.0f - (255.0f * hue_pos);
		B = 0.0f;
	}

	if (100 != sat_percent)
	{
		if (0 == sat_percent)
		{
			R = 255.0f;
			G = 255.0f;
			B = 255.0f;
		}
		else
		{
			if (255.0f != R)
				R += ((255.0f - R) / 100.0f) * (100.0f - sat_percent);
			if (255.0f != G)
				G += ((255.0f - G) / 100.0f) * (100.0f - sat_percent);
			if (255.0f != B)
				B += ((255.0f - B) / 100.0f) * (100.0f - sat_percent);
		}
	}

	if (100 != bri_percent)
	{
		if (0 == bri_percent)
		{
			R = 0.0f;
			G = 0.0f;
			B = 0.0f;
		}
		else
		{
			if (0.0f != R)
				R *= static_cast<float>(bri_percent) / 100.0f;
			if (0.0f != G)
				G *= static_cast<float>(bri_percent) / 100.0f;
			if (0.0f != B)
				B *= static_cast<float>(bri_percent) / 100.0f;
		}
	}

	if (R < 0.0f)
		R = 0.0f;
	else if (R > 255.0f)
		R = 255.0f;

	if (G < 0.0f)
		G = 0.0f;
	else if (G > 255.0f)
		G = 255.0f;

	if (B < 0.0f)
		B = 0.0f;
	else if (B > 255.0f)
		B = 255.0f;

	RGB rgb;

	rgb.r = static_cast<unsigned char>(R);
	rgb.g = static_cast<unsigned char>(G);
	rgb.b = static_cast<unsigned char>(B);

	return rgb;
}

void refresh_vertex_data_blue(void)
{
	vertex_data.clear();

	for (size_t i = 0; i < triangles.size(); i++)
	{
		if (stop)
		{
			vertex_data.clear();
			return;
		}

		vertex_3 colour(0.0f, 0.8f, 1.0f);

		size_t v0_index = triangles[i].vertex[0].index;
		size_t v1_index = triangles[i].vertex[1].index;
		size_t v2_index = triangles[i].vertex[2].index;

		vertex_3 v0_fn(vertices_with_face_normals[v0_index].nx, vertices_with_face_normals[v0_index].ny, vertices_with_face_normals[v0_index].nz);
		vertex_3 v1_fn(vertices_with_face_normals[v1_index].nx, vertices_with_face_normals[v1_index].ny, vertices_with_face_normals[v1_index].nz);
		vertex_3 v2_fn(vertices_with_face_normals[v2_index].nx, vertices_with_face_normals[v2_index].ny, vertices_with_face_normals[v2_index].nz);

		vertex_3 v0(triangles[i].vertex[0].x, triangles[i].vertex[0].y, triangles[i].vertex[0].z);
		vertex_3 v1(triangles[i].vertex[1].x, triangles[i].vertex[1].y, triangles[i].vertex[1].z);
		vertex_3 v2(triangles[i].vertex[2].x, triangles[i].vertex[2].y, triangles[i].vertex[2].z);

		vertex_data.push_back(v0.x);
		vertex_data.push_back(v0.y);
		vertex_data.push_back(v0.z);
		vertex_data.push_back(v0_fn.x);
		vertex_data.push_back(v0_fn.y);
		vertex_data.push_back(v0_fn.z);
		vertex_data.push_back(colour.x);
		vertex_data.push_back(colour.y);
		vertex_data.push_back(colour.z);

		vertex_data.push_back(v1.x);
		vertex_data.push_back(v1.y);
		vertex_data.push_back(v1.z);
		vertex_data.push_back(v1_fn.x);
		vertex_data.push_back(v1_fn.y);
		vertex_data.push_back(v1_fn.z);
		vertex_data.push_back(colour.x);
		vertex_data.push_back(colour.y);
		vertex_data.push_back(colour.z);

		vertex_data.push_back(v2.x);
		vertex_data.push_back(v2.y);
		vertex_data.push_back(v2.z);
		vertex_data.push_back(v2_fn.x);
		vertex_data.push_back(v2_fn.y);
		vertex_data.push_back(v2_fn.z);
		vertex_data.push_back(colour.x);
		vertex_data.push_back(colour.y);
		vertex_data.push_back(colour.z);
	}

}



void refresh_vertex_data_rainbow(void)
{
	vertex_data.clear();

	float min_3d_length = FLT_MAX;
	float max_3d_length = FLT_MIN;

	for (size_t i = 0; i < triangles.size(); i++)
	{
		if (stop)
			return;

		size_t v0_index = triangles[i].vertex[0].index;
		size_t v1_index = triangles[i].vertex[1].index;
		size_t v2_index = triangles[i].vertex[2].index;

		vertex_3 v0(triangles[i].vertex[0].x, triangles[i].vertex[0].y, triangles[i].vertex[0].z);
		vertex_3 v1(triangles[i].vertex[1].x, triangles[i].vertex[1].y, triangles[i].vertex[1].z);
		vertex_3 v2(triangles[i].vertex[2].x, triangles[i].vertex[2].y, triangles[i].vertex[2].z);

		float vertex_length = v0.length();

		if (vertex_length > max_3d_length)
			max_3d_length = vertex_length;

		if (vertex_length < min_3d_length)
			min_3d_length = vertex_length;

		vertex_length = v1.length();

		if (vertex_length > max_3d_length)
			max_3d_length = vertex_length;

		if (vertex_length < min_3d_length)
			min_3d_length = vertex_length;

		vertex_length = v2.length();

		if (vertex_length > max_3d_length)
			max_3d_length = vertex_length;

		if (vertex_length < min_3d_length)
			min_3d_length = vertex_length;
	}

	double max_rainbow = 360.0;
	double min_rainbow = 360.0;





	for (size_t i = 0; i < triangles.size(); i++)
	{
		if (stop)
		{
			vertex_data.clear();
			return;
		}

		vertex_3 colour(1.0f, 0.5f, 0.0);

		size_t v0_index = triangles[i].vertex[0].index;
		size_t v1_index = triangles[i].vertex[1].index;
		size_t v2_index = triangles[i].vertex[2].index;

		vertex_3 v0_fn(vertices_with_face_normals[v0_index].nx, vertices_with_face_normals[v0_index].ny, vertices_with_face_normals[v0_index].nz);
		vertex_3 v1_fn(vertices_with_face_normals[v1_index].nx, vertices_with_face_normals[v1_index].ny, vertices_with_face_normals[v1_index].nz);
		vertex_3 v2_fn(vertices_with_face_normals[v2_index].nx, vertices_with_face_normals[v2_index].ny, vertices_with_face_normals[v2_index].nz);

		vertex_3 v0(triangles[i].vertex[0].x, triangles[i].vertex[0].y, triangles[i].vertex[0].z);
		vertex_3 v1(triangles[i].vertex[1].x, triangles[i].vertex[1].y, triangles[i].vertex[1].z);
		vertex_3 v2(triangles[i].vertex[2].x, triangles[i].vertex[2].y, triangles[i].vertex[2].z);

		float vertex_length = v0.length() - min_3d_length;

		RGB rgb = HSBtoRGB(static_cast<unsigned short int>(
			max_rainbow - ((vertex_length / (max_3d_length - min_3d_length)) * min_rainbow)),
			static_cast<unsigned char>(50),
			static_cast<unsigned char>(100));

		colour.x = rgb.r / 255.0f;
		colour.y = rgb.g / 255.0f;
		colour.z = rgb.b / 255.0f;

		vertex_data.push_back(v0.x);
		vertex_data.push_back(v0.y);
		vertex_data.push_back(v0.z);
		vertex_data.push_back(v0_fn.x);
		vertex_data.push_back(v0_fn.y);
		vertex_data.push_back(v0_fn.z);
		vertex_data.push_back(colour.x);
		vertex_data.push_back(colour.y);
		vertex_data.push_back(colour.z);

		vertex_length = v1.length() - min_3d_length;

		rgb = HSBtoRGB(static_cast<unsigned short int>(
			max_rainbow - ((vertex_length / (max_3d_length - min_3d_length)) * min_rainbow)),
			static_cast<unsigned char>(50),
			static_cast<unsigned char>(100));

		colour.x = rgb.r / 255.0f;
		colour.y = rgb.g / 255.0f;
		colour.z = rgb.b / 255.0f;

		vertex_data.push_back(v1.x);
		vertex_data.push_back(v1.y);
		vertex_data.push_back(v1.z);
		vertex_data.push_back(v1_fn.x);
		vertex_data.push_back(v1_fn.y);
		vertex_data.push_back(v1_fn.z);
		vertex_data.push_back(colour.x);
		vertex_data.push_back(colour.y);
		vertex_data.push_back(colour.z);


		vertex_length = v2.length() - min_3d_length;

		rgb = HSBtoRGB(static_cast<unsigned short int>(
			max_rainbow - ((vertex_length / (max_3d_length - min_3d_length)) * min_rainbow)),
			static_cast<unsigned char>(50),
			static_cast<unsigned char>(100));

		colour.x = rgb.r / 255.0f;
		colour.y = rgb.g / 255.0f;
		colour.z = rgb.b / 255.0f;

		vertex_data.push_back(v2.x);
		vertex_data.push_back(v2.y);
		vertex_data.push_back(v2.z);
		vertex_data.push_back(v2_fn.x);
		vertex_data.push_back(v2_fn.y);
		vertex_data.push_back(v2_fn.z);
		vertex_data.push_back(colour.x);
		vertex_data.push_back(colour.y);
		vertex_data.push_back(colour.z);
	}

}

void refresh_vertex_data(void)
{
	ostringstream oss;

	oss.clear();
	oss.str("");
	oss << "Refreshing vertex data";
	thread_mutex.lock();
	log_system.add_string_to_contents(oss.str());
	thread_mutex.unlock();

	int do_rainbow = rainbow_colouring_checkbox->get_int_val();

	if (do_rainbow)
		refresh_vertex_data_rainbow();
	else
		refresh_vertex_data_blue();

	if (stop)
	{
		oss.clear();
		oss.str("");
		oss << "Cancelled refreshing vertex data";
		thread_mutex.lock();
		log_system.add_string_to_contents(oss.str());
		thread_mutex.unlock();
	}
	else
	{
		oss.clear();
		oss.str("");
		oss << "Done refreshing vertex data";
		thread_mutex.lock();
		log_system.add_string_to_contents(oss.str());
		thread_mutex.unlock();
	}
}


void myGlutIdle(void)
{
	if (false == thread_is_running && false == generate_button)
	{
		if (false == vertex_data_refreshed && false == stop && triangles.size() > 0)
		{
			refresh_vertex_data();
			vertex_data_refreshed = true;
		}

		generate_button = true;
		generate_mesh_button->set_name(const_cast<char*>("Generate mesh"));
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

	glGenBuffers(1, &triangle_buffer);

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
	
	if (vertex_data_refreshed && vertex_data.size() > 0)
	{
		const GLuint components_per_vertex = 9;
		const GLuint components_per_normal = 3;
		const GLuint components_per_position = 3;
		const GLuint components_per_colour = 3;

		const GLuint num_vertices = static_cast<GLuint>(vertex_data.size()) / components_per_vertex;

		glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(GLfloat), &vertex_data[0], GL_DYNAMIC_DRAW);

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


	if (log_system.get_contents_size() > 0)
	{
		size_t char_x_pos = 10;
		size_t char_y_pos = 30;

		RGB text_colour;
		text_colour.r = 255;
		text_colour.g = 255;
		text_colour.b = 255;

		vector<unsigned char> fbpixels(4 * static_cast<size_t>(win_x) * static_cast<size_t>(win_y));

		glReadPixels(0, 0, win_x, win_y, GL_RGBA, GL_UNSIGNED_BYTE, &fbpixels[0]);

		thread_mutex.lock();
		for (size_t i = 0; i < log_system.get_contents_size(); i++)
		{
			string s;
			log_system.get_string_from_contents(i, s);
			print_sentence(fbpixels, win_x, win_y, char_x_pos, char_y_pos, s, text_colour);
			char_y_pos += 20;
		}
		thread_mutex.unlock();

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

	equation_edittext = glui->add_edittext(const_cast<char*>("Equation:"), 0, const_cast<char*>("Z = Z*Z + C"), 3, control_cb);
	equation_edittext->set_text("Z = Z*Z + C");
	equation_edittext->set_w(150);

	glui->add_separator();
rainbow_colouring_checkbox
	 = glui->add_checkbox("Rainbow colouring");
	randomize_c_checkbox = glui->add_checkbox("Randomize C");
	use_pedestal_checkbox = glui->add_checkbox("Use pedestal");
	use_pedestal_checkbox->set_int_val(1);

	pedestal_y_start_edittext = glui->add_edittext(const_cast<char*>("Pedestal y start:"), 0, const_cast<char*>("1.0"), 3, control_cb);
	pedestal_y_start_edittext->set_text("0.0");

	pedestal_y_end_edittext = glui->add_edittext(const_cast<char*>("Pedestal y end:"), 0, const_cast<char*>("1.0"), 3, control_cb);
	pedestal_y_end_edittext->set_text("0.1");


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