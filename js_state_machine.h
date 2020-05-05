#ifndef JS_STATE_MACHINE_H
#define JS_STATE_MACHINE_H

#include "fractal_set_parameters.h"
#include "primitives.h"
#include "eqparse.h"
#include "logging_system.h"
#include "marching_cubes.h"
using namespace marching_cubes;

#include "vertex_geometry_fragment_shader.h"
#include "vertex_geometry_shader.h"


#include <vector>
using std::vector;
#include <fstream>
using std::ofstream;
using std::ifstream;

#include<string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include <mutex>
using std::mutex;

#include <set>
using std::set;

#include <sstream>
using std::ostringstream;



bool compile_and_link_compute_shader(const char* const file_name, GLuint& program, logging_system& ls);




class js_state_machine;
typedef int (js_state_machine::* js_state_machine_member_function_pointer)(void);


#define STATE_UNINITIALIZED 0
#define STATE_FINISHED 1
#define STATE_CANCELLED 2
#define STATE_G0_STAGE_0 3
#define STATE_G0_STAGE_1 4
#define STATE_G1_STAGE_0 5
#define STATE_G1_STAGE_1 5
#define STATE_G1_STAGE_2 6
#define STATE_G1_STAGE_3 7
#define STATE_G1_STAGE_4 8
#define STATE_G1_STAGE_5 9
#define STATE_G2_STAGE_0 10
#define STATE_G2_STAGE_1 11
#define STATE_G3_STAGE_0 12
#define STATE_G3_STAGE_1 13


class RGB
{
public:
	unsigned char r, g, b;
};





RGB HSBtoRGB(unsigned short int hue_degree, unsigned char sat_percent, unsigned char bri_percent);


class js_state_machine
{
public:
	vector<float> vertex_data;

	size_t get_burst_length(void)
	{
		return fsp.burst_length;
	}

	js_state_machine(void);

	~js_state_machine(void);

	size_t get_state(void);
	bool init(fractal_set_parameters& fsp_in, logging_system* ls);
	void cancel(void);
	void proceed(void);

	std::chrono::high_resolution_clock::time_point start_time, end_time;

	GLuint mc_shader = 0;

	bool tesselate_adjacent_xy_plane_pair_cpu(size_t& box_count, const vector<float>& xyplane0, const vector<float>& xyplane1, const size_t z, vector<triangle>& triangles, const float isovalue, const float x_grid_min, const float x_grid_max, const size_t x_res, const float y_grid_min, const float y_grid_max, const size_t y_res, const float z_grid_min, const float z_grid_max, const size_t z_res)
	{
		const float x_step_size = (x_grid_max - x_grid_min) / (x_res - 1);
		const float y_step_size = (y_grid_max - y_grid_min) / (y_res - 1);
		const float z_step_size = (z_grid_max - z_grid_min) / (z_res - 1);

		for (size_t x = 0; x < x_res - 1; x++)
		{
			for (size_t y = 0; y < y_res - 1; y++)
			{
				grid_cube temp_cube;

				size_t x_offset = 0;
				size_t y_offset = 0;
				size_t z_offset = 0;

				// Setup vertex 0
				x_offset = 0;
				y_offset = 0;
				z_offset = 0;
				temp_cube.vertex[0].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[0].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[0].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[0] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[0] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Setup vertex 1
				x_offset = 1;
				y_offset = 0;
				z_offset = 0;
				temp_cube.vertex[1].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[1].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[1].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[1] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[1] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Setup vertex 2
				x_offset = 1;
				y_offset = 0;
				z_offset = 1;
				temp_cube.vertex[2].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[2].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[2].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[2] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[2] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Setup vertex 3
				x_offset = 0;
				y_offset = 0;
				z_offset = 1;
				temp_cube.vertex[3].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[3].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[3].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[3] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[3] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Setup vertex 4
				x_offset = 0;
				y_offset = 1;
				z_offset = 0;
				temp_cube.vertex[4].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[4].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[4].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[4] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[4] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Setup vertex 5
				x_offset = 1;
				y_offset = 1;
				z_offset = 0;
				temp_cube.vertex[5].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[5].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[5].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[5] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[5] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Setup vertex 6
				x_offset = 1;
				y_offset = 1;
				z_offset = 1;
				temp_cube.vertex[6].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[6].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[6].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[6] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[6] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Setup vertex 7
				x_offset = 0;
				y_offset = 1;
				z_offset = 1;
				temp_cube.vertex[7].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[7].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[7].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[7] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[7] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Generate triangles from cube.
				static triangle temp_triangle_array[5];

				short unsigned int number_of_triangles_generated = tesselate_grid_cube(isovalue, temp_cube, temp_triangle_array);

				if (number_of_triangles_generated > 0)
					box_count++;

				for (short unsigned int i = 0; i < number_of_triangles_generated; i++)
					triangles.push_back(temp_triangle_array[i]);
			}
		}

		return true;
	}

	bool tesselate_adjacent_xy_plane_pair_gpu(size_t& box_count, const vector<float>& xyplane0, const vector<float>& xyplane1, const size_t z, vector<triangle>& triangles, const float isovalue, const float x_grid_min, const float x_grid_max, const size_t x_res, const float y_grid_min, const float y_grid_max, const size_t y_res, const float z_grid_min, const float z_grid_max, const size_t z_res)
	{
		vertex_geometry_shader points;

		if (false == points.init("points.vs.glsl", "points.gs.glsl"))
		{
			cout << "Could not load points shader" << endl;
			return false;
		}
		
		glUseProgram(points.get_program());
		glEnable(GL_RASTERIZER_DISCARD);
		vector<float> points_vertex_data;

		const float x_step_size = (x_grid_max - x_grid_min) / (x_res - 1);
		const float y_step_size = (y_grid_max - y_grid_min) / (y_res - 1);
		const float z_step_size = (z_grid_max - z_grid_min) / (z_res - 1);

		for (size_t x = 0; x < x_res - 1; x++)
		{
			for (size_t y = 0; y < y_res - 1; y++)
			{
				grid_cube temp_cube;

				size_t x_offset = 0;
				size_t y_offset = 0;
				size_t z_offset = 0;

				// Setup vertex 0
				x_offset = 0;
				y_offset = 0;
				z_offset = 0;
				temp_cube.vertex[0].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[0].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[0].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[0] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[0] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Setup vertex 1
				x_offset = 1;
				y_offset = 0;
				z_offset = 0;
				temp_cube.vertex[1].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[1].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[1].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[1] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[1] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Setup vertex 2
				x_offset = 1;
				y_offset = 0;
				z_offset = 1;
				temp_cube.vertex[2].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[2].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[2].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[2] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[2] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Setup vertex 3
				x_offset = 0;
				y_offset = 0;
				z_offset = 1;
				temp_cube.vertex[3].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[3].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[3].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[3] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[3] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Setup vertex 4
				x_offset = 0;
				y_offset = 1;
				z_offset = 0;
				temp_cube.vertex[4].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[4].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[4].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[4] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[4] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Setup vertex 5
				x_offset = 1;
				y_offset = 1;
				z_offset = 0;
				temp_cube.vertex[5].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[5].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[5].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[5] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[5] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Setup vertex 6
				x_offset = 1;
				y_offset = 1;
				z_offset = 1;
				temp_cube.vertex[6].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[6].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[6].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[6] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[6] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				// Setup vertex 7
				x_offset = 0;
				y_offset = 1;
				z_offset = 1;
				temp_cube.vertex[7].x = x_grid_min + ((x + x_offset) * x_step_size);
				temp_cube.vertex[7].y = y_grid_min + ((y + y_offset) * y_step_size);
				temp_cube.vertex[7].z = z_grid_min + ((z + z_offset) * z_step_size);

				if (0 == z_offset)
					temp_cube.value[7] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
				else
					temp_cube.value[7] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

				points_vertex_data.push_back(temp_cube.vertex[0].x);
				points_vertex_data.push_back(temp_cube.vertex[0].y);
				points_vertex_data.push_back(temp_cube.vertex[0].z);
				points_vertex_data.push_back(temp_cube.value[0]);

				points_vertex_data.push_back(temp_cube.vertex[1].x);
				points_vertex_data.push_back(temp_cube.vertex[1].y);
				points_vertex_data.push_back(temp_cube.vertex[1].z);
				points_vertex_data.push_back(temp_cube.value[1]);

				points_vertex_data.push_back(temp_cube.vertex[2].x);
				points_vertex_data.push_back(temp_cube.vertex[2].y);
				points_vertex_data.push_back(temp_cube.vertex[2].z);
				points_vertex_data.push_back(temp_cube.value[2]);

				points_vertex_data.push_back(temp_cube.vertex[3].x);
				points_vertex_data.push_back(temp_cube.vertex[3].y);
				points_vertex_data.push_back(temp_cube.vertex[3].z);
				points_vertex_data.push_back(temp_cube.value[3]);

				points_vertex_data.push_back(temp_cube.vertex[4].x);
				points_vertex_data.push_back(temp_cube.vertex[4].y);
				points_vertex_data.push_back(temp_cube.vertex[4].z);
				points_vertex_data.push_back(temp_cube.value[4]);

				points_vertex_data.push_back(temp_cube.vertex[5].x);
				points_vertex_data.push_back(temp_cube.vertex[5].y);
				points_vertex_data.push_back(temp_cube.vertex[5].z);
				points_vertex_data.push_back(temp_cube.value[5]);

				points_vertex_data.push_back(temp_cube.vertex[6].x);
				points_vertex_data.push_back(temp_cube.vertex[6].y);
				points_vertex_data.push_back(temp_cube.vertex[6].z);
				points_vertex_data.push_back(temp_cube.value[6]);

				points_vertex_data.push_back(temp_cube.vertex[7].x);
				points_vertex_data.push_back(temp_cube.vertex[7].y);
				points_vertex_data.push_back(temp_cube.vertex[7].z);
				points_vertex_data.push_back(temp_cube.value[7]);
			}
		}


//	https://github.com/progschj/OpenGL-Examples/blob/master/09transform_feedback.cpp



		const GLuint components_per_position = 4;
		const GLuint components_per_vertex = 8 * components_per_position;

		static GLuint triangle_buffer;

		glDeleteBuffers(1, &triangle_buffer);
		glGenBuffers(1, &triangle_buffer);

		const GLuint num_vertices = static_cast<GLuint>(points_vertex_data.size()) / components_per_vertex;

		glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer);
		glBufferData(GL_ARRAY_BUFFER, points_vertex_data.size() * sizeof(GLfloat), &points_vertex_data[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(glGetAttribLocation(points.get_program(), "position0"));
		glVertexAttribPointer(glGetAttribLocation(points.get_program(), "position0"),
			components_per_position,
			GL_FLOAT,
			GL_FALSE,
			components_per_vertex * sizeof(GLfloat),
			0);

		glEnableVertexAttribArray(glGetAttribLocation(points.get_program(), "position1"));
		glVertexAttribPointer(glGetAttribLocation(points.get_program(), "position1"),
			components_per_position,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(1 * components_per_position * sizeof(GLfloat)));

		glEnableVertexAttribArray(glGetAttribLocation(points.get_program(), "position2"));
		glVertexAttribPointer(glGetAttribLocation(points.get_program(), "position2"),
			components_per_position,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(2 * components_per_position * sizeof(GLfloat)));

		glEnableVertexAttribArray(glGetAttribLocation(points.get_program(), "position3"));
		glVertexAttribPointer(glGetAttribLocation(points.get_program(), "position3"),
			components_per_position,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(3 * components_per_position * sizeof(GLfloat)));

		glEnableVertexAttribArray(glGetAttribLocation(points.get_program(), "position4"));
		glVertexAttribPointer(glGetAttribLocation(points.get_program(), "position4"),
			components_per_position,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(4 * components_per_position * sizeof(GLfloat)));

		glEnableVertexAttribArray(glGetAttribLocation(points.get_program(), "position5"));
		glVertexAttribPointer(glGetAttribLocation(points.get_program(), "position5"),
			components_per_position,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(5 * components_per_position * sizeof(GLfloat)));

		glEnableVertexAttribArray(glGetAttribLocation(points.get_program(), "position6"));
		glVertexAttribPointer(glGetAttribLocation(points.get_program(), "position6"),
			components_per_position,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(6 * components_per_position * sizeof(GLfloat)));

		glEnableVertexAttribArray(glGetAttribLocation(points.get_program(), "position7"));
		glVertexAttribPointer(glGetAttribLocation(points.get_program(), "position7"),
			components_per_position,	
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(7 * components_per_position * sizeof(GLfloat)));
		



		glEnable(GL_RASTERIZER_DISCARD);

		GLuint array_buffer_vbo, feedback_buffer_vbo;
		glGenBuffers(1, &array_buffer_vbo);
		glGenBuffers(1, &feedback_buffer_vbo);

vector<float> in_data((fsp.resolution - 1)* (fsp.resolution - 1) * 5 * 9, 0.0f);
vector<float> out_data = in_data;

		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, feedback_buffer_vbo);
		glNamedBufferStorage(feedback_buffer_vbo, out_data.size() * sizeof(float), &out_data[0], GL_DYNAMIC_STORAGE_BIT);
		
		//glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, out_data.size() * sizeof(float), &out_data[0], GL_DYNAMIC_COPY);


		glBindBuffer(GL_ARRAY_BUFFER, array_buffer_vbo);
		glNamedBufferStorage(array_buffer_vbo, in_data.size() * sizeof(float), &in_data[0], GL_DYNAMIC_STORAGE_BIT);


		//glBufferData(GL_ARRAY_BUFFER, in_data.size() * sizeof(float), &in_data[0], GL_DYNAMIC_COPY);

		//glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, feedback_buffer_vbo);
		//glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, out_data.size() * sizeof(float), &out_data[0], GL_DYNAMIC_COPY);

		GLuint query;

		glGenQueries(1, &query);

		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
			
		glBeginTransformFeedback(GL_TRIANGLES);
			glDrawArrays(GL_POINTS, 0, num_vertices);
		glEndTransformFeedback();

		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

		GLuint primitives;
		glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);

		printf("%u primitives written!\n\n", primitives);



		//glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, out_data.size() * sizeof(float), &out_data[0]);
		glGetNamedBufferSubData(feedback_buffer_vbo, 0, primitives*9*sizeof(float), &out_data[0]);

		glDisable(GL_RASTERIZER_DISCARD);


		for (size_t i = 0; i < primitives; i++)
		{
			size_t feedback_index = 9 * i;

			triangle t;

			t.vertex[0].x = out_data[feedback_index + 0];
			t.vertex[0].y = out_data[feedback_index + 1];
			t.vertex[0].z = out_data[feedback_index + 2];

			t.vertex[1].x = out_data[feedback_index + 3];
			t.vertex[1].y = out_data[feedback_index + 4];
			t.vertex[1].z = out_data[feedback_index + 5];

			t.vertex[2].x = out_data[feedback_index + 6];
			t.vertex[2].y = out_data[feedback_index + 7];
			t.vertex[2].z = out_data[feedback_index + 8];

			triangles.push_back(t);
		}























/*
		GLuint query;

		glGenQueries(1, &query);

		glBeginQuery(GL_PRIMITIVES_GENERATED, query);

		vector<float> in_data((fsp.resolution - 1)* (fsp.resolution - 1) * 5 * 9);

		GLuint tbo;
		glGenBuffers(1, &tbo);
		glBindBuffer(GL_ARRAY_BUFFER, tbo);
		glBufferData(GL_ARRAY_BUFFER, in_data.size() * sizeof(float), &in_data[0], GL_STATIC_READ);

		glBeginTransformFeedback(GL_TRIANGLES);
			glDrawArrays(GL_POINTS, 0, num_vertices);	
		glEndTransformFeedback();

		glEndQuery(GL_PRIMITIVES_GENERATED);
	
		GLuint primitives;
		glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);

		vector<float> feedback(in_data.size(), 0.0f);
		glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, feedback.size()*sizeof(float), &feedback[0]);

		for (size_t i = 0; i < primitives; i++)
		{
			size_t feedback_index = 9 * i;

			triangle t;

			t.vertex[0].x = feedback[feedback_index + 0];
			t.vertex[0].y = feedback[feedback_index + 1];
			t.vertex[0].z = feedback[feedback_index + 2];

			t.vertex[1].x = feedback[feedback_index + 3];
			t.vertex[1].y = feedback[feedback_index + 4];
			t.vertex[1].z = feedback[feedback_index + 5];

			t.vertex[2].x = feedback[feedback_index + 6];
			t.vertex[2].y = feedback[feedback_index + 7];
			t.vertex[2].z = feedback[feedback_index + 8];

			triangles.push_back(t);
		}

		*/

		
		//for (int i = 0; i < 15; i++) {
		//	printf("%f\n", feedback[i]);
		//}




		// read	triangles via GPU to CPU copy (transform feedback)

		//GLint queryResult = 0;
		//glEndQuery(GL_PRIMITIVES_GENERATED);

		//glGetQueryObjectiv(query, GL_QUERY_RESULT, &queryResult);`

		//printf("Primitives count: %d\n", queryResult);


		//// https://open.gl/feedback
		// https://www.reddit.com/r/opengl/comments/5wa3kv/transform_feedback_question/
		// http://www.java-gaming.org/topics/opengl-transform-feedback/27786/view.html




		//glDisable(GL_RASTERIZER_DISCARD);


		return true;
	}



/*
bool tesselate_adjacent_xy_plane_pair_gpu(size_t& box_count, const vector<float>& xyplane0, const vector<float>& xyplane1, const size_t z, vector<triangle>& triangles, const float isovalue, const float x_grid_min, const float x_grid_max, const size_t x_res, const float y_grid_min, const float y_grid_max, const size_t y_res, const float z_grid_min, const float z_grid_max, const size_t z_res)
{
	const float x_step_size = (x_grid_max - x_grid_min) / (x_res - 1);
	const float y_step_size = (y_grid_max - y_grid_min) / (y_res - 1);
	const float z_step_size = (z_grid_max - z_grid_min) / (z_res - 1);

	GLsizei width = static_cast<GLsizei>(x_res) - 1;
	GLsizei height = static_cast<GLsizei>(y_res) - 1;

	glUseProgram(mc_shader);

	vector<GLfloat> input_tex0_data(4 * width * height);
	vector<GLfloat> input_tex1_data(4 * width * height);
	vector<GLfloat> input_tex2_data(4 * width * height);
	vector<GLfloat> input_tex3_data(4 * width * height);
	vector<GLfloat> input_tex4_data(4 * width * height);
	vector<GLfloat> input_tex5_data(4 * width * height);
	vector<GLfloat> input_tex6_data(4 * width * height);
	vector<GLfloat> input_tex7_data(4 * width * height);

	vector<GLfloat> output_tri_count_data(width * height);

	vector<GLfloat> output_tex0_data(3 * width * height);
	vector<GLfloat> output_tex1_data(3 * width * height);
	vector<GLfloat> output_tex2_data(3 * width * height);
	vector<GLfloat> output_tex3_data(3 * width * height);
	vector<GLfloat> output_tex4_data(3 * width * height);
	vector<GLfloat> output_tex5_data(3 * width * height);
	vector<GLfloat> output_tex6_data(3 * width * height);
	vector<GLfloat> output_tex7_data(3 * width * height);
	vector<GLfloat> output_tex8_data(3 * width * height);
	vector<GLfloat> output_tex9_data(3 * width * height);
	vector<GLfloat> output_tex10_data(3 * width * height);
	vector<GLfloat> output_tex11_data(3 * width * height);
	vector<GLfloat> output_tex12_data(3 * width * height);
	vector<GLfloat> output_tex13_data(3 * width * height);
	vector<GLfloat> output_tex14_data(3 * width * height);

	GLuint input_tex0, input_tex1, input_tex2, input_tex3, input_tex4, input_tex5, input_tex6, input_tex7;

	glGenTextures(1, &input_tex0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, input_tex0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &input_tex1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, input_tex1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &input_tex2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, input_tex2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &input_tex3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, input_tex3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &input_tex4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, input_tex4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &input_tex5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, input_tex5);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &input_tex6);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, input_tex6);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &input_tex7);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, input_tex7);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	GLuint output_triangle_number_tex;

	glGenTextures(1, &output_triangle_number_tex);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, output_triangle_number_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glBindImageTexture(8, output_triangle_number_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

	GLuint output_tex0, output_tex1, output_tex2, output_tex3, output_tex4, output_tex5, output_tex6,
		output_tex7, output_tex8, output_tex9, output_tex10, output_tex11, output_tex12, output_tex13,
		output_tex14;

	glGenTextures(1, &output_tex0);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, output_tex0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(9, output_tex0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);

	glGenTextures(1, &output_tex1);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, output_tex1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(10, output_tex1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);

	glGenTextures(1, &output_tex2);
	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, output_tex2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(11, output_tex2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);

	glGenTextures(1, &output_tex3);
	glActiveTexture(GL_TEXTURE12);
	glBindTexture(GL_TEXTURE_2D, output_tex3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(12, output_tex3, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);

	glGenTextures(1, &output_tex4);
	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, output_tex4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(13, output_tex4, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);

	glGenTextures(1, &output_tex5);
	glActiveTexture(GL_TEXTURE14);
	glBindTexture(GL_TEXTURE_2D, output_tex5);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(14, output_tex5, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);

	glGenTextures(1, &output_tex6);
	glActiveTexture(GL_TEXTURE15);
	glBindTexture(GL_TEXTURE_2D, output_tex6);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(15, output_tex6, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);

	glGenTextures(1, &output_tex7);
	glActiveTexture(GL_TEXTURE16);
	glBindTexture(GL_TEXTURE_2D, output_tex7);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(16, output_tex7, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);

	glGenTextures(1, &output_tex8);
	glActiveTexture(GL_TEXTURE17);
	glBindTexture(GL_TEXTURE_2D, output_tex8);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(17, output_tex8, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);

	glGenTextures(1, &output_tex9);
	glActiveTexture(GL_TEXTURE18);
	glBindTexture(GL_TEXTURE_2D, output_tex9);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(18, output_tex9, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);

	glGenTextures(1, &output_tex10);
	glActiveTexture(GL_TEXTURE19);
	glBindTexture(GL_TEXTURE_2D, output_tex10);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(19, output_tex10, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);

	glGenTextures(1, &output_tex11);
	glActiveTexture(GL_TEXTURE20);
	glBindTexture(GL_TEXTURE_2D, output_tex11);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(20, output_tex11, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);

	glGenTextures(1, &output_tex12);
	glActiveTexture(GL_TEXTURE21);
	glBindTexture(GL_TEXTURE_2D, output_tex12);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(21, output_tex12, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);

	glGenTextures(1, &output_tex13);
	glActiveTexture(GL_TEXTURE22);
	glBindTexture(GL_TEXTURE_2D, output_tex13);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(22, output_tex13, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);

	glGenTextures(1, &output_tex14);
	glActiveTexture(GL_TEXTURE23);
	glBindTexture(GL_TEXTURE_2D, output_tex14);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindImageTexture(23, output_tex14, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);


	size_t index = 0;

	for (size_t x = 0; x < width; x++)
	{
		for (size_t y = 0; y < height; y++)
		{
			grid_cube temp_cube;

			size_t x_offset = 0;
			size_t y_offset = 0;
			size_t z_offset = 0;

			// Setup vertex 0
			x_offset = 0;
			y_offset = 0;
			z_offset = 0;
			temp_cube.vertex[0].x = x_grid_min + ((x + x_offset) * x_step_size);
			temp_cube.vertex[0].y = y_grid_min + ((y + y_offset) * y_step_size);
			temp_cube.vertex[0].z = z_grid_min + ((z + z_offset) * z_step_size);

			if (0 == z_offset)
				temp_cube.value[0] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
			else
				temp_cube.value[0] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

			// Setup vertex 1
			x_offset = 1;
			y_offset = 0;
			z_offset = 0;
			temp_cube.vertex[1].x = x_grid_min + ((x + x_offset) * x_step_size);
			temp_cube.vertex[1].y = y_grid_min + ((y + y_offset) * y_step_size);
			temp_cube.vertex[1].z = z_grid_min + ((z + z_offset) * z_step_size);

			if (0 == z_offset)
				temp_cube.value[1] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
			else
				temp_cube.value[1] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

			// Setup vertex 2
			x_offset = 1;
			y_offset = 0;
			z_offset = 1;
			temp_cube.vertex[2].x = x_grid_min + ((x + x_offset) * x_step_size);
			temp_cube.vertex[2].y = y_grid_min + ((y + y_offset) * y_step_size);
			temp_cube.vertex[2].z = z_grid_min + ((z + z_offset) * z_step_size);

			if (0 == z_offset)
				temp_cube.value[2] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
			else
				temp_cube.value[2] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

			// Setup vertex 3
			x_offset = 0;
			y_offset = 0;
			z_offset = 1;
			temp_cube.vertex[3].x = x_grid_min + ((x + x_offset) * x_step_size);
			temp_cube.vertex[3].y = y_grid_min + ((y + y_offset) * y_step_size);
			temp_cube.vertex[3].z = z_grid_min + ((z + z_offset) * z_step_size);

			if (0 == z_offset)
				temp_cube.value[3] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
			else
				temp_cube.value[3] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

			// Setup vertex 4
			x_offset = 0;
			y_offset = 1;
			z_offset = 0;
			temp_cube.vertex[4].x = x_grid_min + ((x + x_offset) * x_step_size);
			temp_cube.vertex[4].y = y_grid_min + ((y + y_offset) * y_step_size);
			temp_cube.vertex[4].z = z_grid_min + ((z + z_offset) * z_step_size);

			if (0 == z_offset)
				temp_cube.value[4] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
			else
				temp_cube.value[4] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

			// Setup vertex 5
			x_offset = 1;
			y_offset = 1;
			z_offset = 0;
			temp_cube.vertex[5].x = x_grid_min + ((x + x_offset) * x_step_size);
			temp_cube.vertex[5].y = y_grid_min + ((y + y_offset) * y_step_size);
			temp_cube.vertex[5].z = z_grid_min + ((z + z_offset) * z_step_size);

			if (0 == z_offset)
				temp_cube.value[5] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
			else
				temp_cube.value[5] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

			// Setup vertex 6
			x_offset = 1;
			y_offset = 1;
			z_offset = 1;
			temp_cube.vertex[6].x = x_grid_min + ((x + x_offset) * x_step_size);
			temp_cube.vertex[6].y = y_grid_min + ((y + y_offset) * y_step_size);
			temp_cube.vertex[6].z = z_grid_min + ((z + z_offset) * z_step_size);

			if (0 == z_offset)
				temp_cube.value[6] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
			else
				temp_cube.value[6] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

			// Setup vertex 7
			x_offset = 0;
			y_offset = 1;
			z_offset = 1;
			temp_cube.vertex[7].x = x_grid_min + ((x + x_offset) * x_step_size);
			temp_cube.vertex[7].y = y_grid_min + ((y + y_offset) * y_step_size);
			temp_cube.vertex[7].z = z_grid_min + ((z + z_offset) * z_step_size);

			if (0 == z_offset)
				temp_cube.value[7] = xyplane0[(x + x_offset) * y_res + (y + y_offset)];
			else
				temp_cube.value[7] = xyplane1[(x + x_offset) * y_res + (y + y_offset)];

			input_tex0_data[index + 0] = temp_cube.vertex[0].x;
			input_tex0_data[index + 1] = temp_cube.vertex[0].y;
			input_tex0_data[index + 2] = temp_cube.vertex[0].z;
			input_tex0_data[index + 3] = temp_cube.value[0];

			input_tex1_data[index + 0] = temp_cube.vertex[1].x;
			input_tex1_data[index + 1] = temp_cube.vertex[1].y;
			input_tex1_data[index + 2] = temp_cube.vertex[1].z;
			input_tex1_data[index + 3] = temp_cube.value[1];

			input_tex2_data[index + 0] = temp_cube.vertex[2].x;
			input_tex2_data[index + 1] = temp_cube.vertex[2].y;
			input_tex2_data[index + 2] = temp_cube.vertex[2].z;
			input_tex2_data[index + 3] = temp_cube.value[2];

			input_tex3_data[index + 0] = temp_cube.vertex[3].x;
			input_tex3_data[index + 1] = temp_cube.vertex[3].y;
			input_tex3_data[index + 2] = temp_cube.vertex[3].z;
			input_tex3_data[index + 3] = temp_cube.value[3];

			input_tex4_data[index + 0] = temp_cube.vertex[4].x;
			input_tex4_data[index + 1] = temp_cube.vertex[4].y;
			input_tex4_data[index + 2] = temp_cube.vertex[4].z;
			input_tex4_data[index + 3] = temp_cube.value[4];

			input_tex5_data[index + 0] = temp_cube.vertex[5].x;
			input_tex5_data[index + 1] = temp_cube.vertex[5].y;
			input_tex5_data[index + 2] = temp_cube.vertex[5].z;
			input_tex5_data[index + 3] = temp_cube.value[5];

			input_tex6_data[index + 0] = temp_cube.vertex[6].x;
			input_tex6_data[index + 1] = temp_cube.vertex[6].y;
			input_tex6_data[index + 2] = temp_cube.vertex[6].z;
			input_tex6_data[index + 3] = temp_cube.value[6];

			input_tex7_data[index + 0] = temp_cube.vertex[7].x;
			input_tex7_data[index + 1] = temp_cube.vertex[7].y;
			input_tex7_data[index + 2] = temp_cube.vertex[7].z;
			input_tex7_data[index + 3] = temp_cube.value[7];

			index += 4;
		}
	}


	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, &input_tex0_data[0]);
	glBindImageTexture(0, input_tex0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glActiveTexture(GL_TEXTURE1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, &input_tex1_data[0]);
	glBindImageTexture(1, input_tex1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glActiveTexture(GL_TEXTURE2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, &input_tex2_data[0]);
	glBindImageTexture(2, input_tex2, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glActiveTexture(GL_TEXTURE3);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, &input_tex3_data[0]);
	glBindImageTexture(3, input_tex3, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glActiveTexture(GL_TEXTURE4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, &input_tex4_data[0]);
	glBindImageTexture(4, input_tex4, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glActiveTexture(GL_TEXTURE5);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, &input_tex5_data[0]);
	glBindImageTexture(5, input_tex5, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glActiveTexture(GL_TEXTURE6);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, &input_tex6_data[0]);
	glBindImageTexture(6, input_tex6, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glActiveTexture(GL_TEXTURE7);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, &input_tex7_data[0]);
	glBindImageTexture(7, input_tex7, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	// Pass in the input image and quaternion Julia set parameters as uniforms
	//glUniform1i(glGetUniformLocation(compute_shader_program, "input_image"), 1); // use GL_TEXTURE1
	//glUniform4f(glGetUniformLocation(compute_shader_program, "c"), C.x, C.y, C.z, C.w);
	//glUniform1i(glGetUniformLocatio	n(compute_shader_program, "max_iterations"), max_iterations);
	//glUniform1f(glGetUniformLocation(compute_shader_program, "threshold"), threshold);

	// Run compute shader
	glDispatchCompute((GLuint)width, (GLuint)height, 1);

	// Wait for compute shader to finish
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	// Copy output pixel array to CPU as texture 8
	glActiveTexture(GL_TEXTURE8);
	glBindImageTexture(8, output_triangle_number_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &output_tri_count_data[0]);

	glActiveTexture(GL_TEXTURE9);
	glBindImageTexture(9, output_tex0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex0_data[0]);

	glActiveTexture(GL_TEXTURE10);
	glBindImageTexture(10, output_tex1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex1_data[0]);

	glActiveTexture(GL_TEXTURE11);
	glBindImageTexture(11, output_tex2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex2_data[0]);

	glActiveTexture(GL_TEXTURE12);
	glBindImageTexture(12, output_tex3, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex3_data[0]);

	glActiveTexture(GL_TEXTURE13);
	glBindImageTexture(13, output_tex4, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex4_data[0]);

	glActiveTexture(GL_TEXTURE14);
	glBindImageTexture(14, output_tex5, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex5_data[0]);

	glActiveTexture(GL_TEXTURE15);
	glBindImageTexture(15, output_tex6, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex6_data[0]);

	glActiveTexture(GL_TEXTURE16);
	glBindImageTexture(16, output_tex7, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex7_data[0]);

	glActiveTexture(GL_TEXTURE17);
	glBindImageTexture(17, output_tex8, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex8_data[0]);

	glActiveTexture(GL_TEXTURE18);
	glBindImageTexture(18, output_tex9, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex9_data[0]);

	glActiveTexture(GL_TEXTURE19);
	glBindImageTexture(19, output_tex10, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex10_data[0]);

	glActiveTexture(GL_TEXTURE20);
	glBindImageTexture(20, output_tex11, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex11_data[0]);

	glActiveTexture(GL_TEXTURE21);
	glBindImageTexture(21, output_tex12, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex12_data[0]);

	glActiveTexture(GL_TEXTURE22);
	glBindImageTexture(22, output_tex13, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex13_data[0]);

	glActiveTexture(GL_TEXTURE23);
	glBindImageTexture(23, output_tex14, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &output_tex14_data[0]);




	for (size_t i = 0; i < output_tri_count_data.size(); i++)
	{
		size_t output_data_index = 3 * i;

		size_t num_tris = static_cast<size_t>(output_tri_count_data[i]);

		if (num_tris == 5)
		{
			triangle t;

			t.vertex[0].x = output_tex0_data[output_data_index + 0];
			t.vertex[0].y = output_tex0_data[output_data_index + 1];
			t.vertex[0].z = output_tex0_data[output_data_index + 2];

			t.vertex[1].x = output_tex1_data[output_data_index + 0];
			t.vertex[1].y = output_tex1_data[output_data_index + 1];
			t.vertex[1].z = output_tex1_data[output_data_index + 2];

			t.vertex[2].x = output_tex2_data[output_data_index + 0];
			t.vertex[2].y = output_tex2_data[output_data_index + 1];
			t.vertex[2].z = output_tex2_data[output_data_index + 2];

			triangles.push_back(t);

			t.vertex[0].x = output_tex3_data[output_data_index + 0];
			t.vertex[0].y = output_tex3_data[output_data_index + 1];
			t.vertex[0].z = output_tex3_data[output_data_index + 2];

			t.vertex[1].x = output_tex4_data[output_data_index + 0];
			t.vertex[1].y = output_tex4_data[output_data_index + 1];
			t.vertex[1].z = output_tex4_data[output_data_index + 2];

			t.vertex[2].x = output_tex5_data[output_data_index + 0];
			t.vertex[2].y = output_tex5_data[output_data_index + 1];
			t.vertex[2].z = output_tex5_data[output_data_index + 2];

			triangles.push_back(t);

			t.vertex[0].x = output_tex6_data[output_data_index + 0];
			t.vertex[0].y = output_tex6_data[output_data_index + 1];
			t.vertex[0].z = output_tex6_data[output_data_index + 2];

			t.vertex[1].x = output_tex7_data[output_data_index + 0];
			t.vertex[1].y = output_tex7_data[output_data_index + 1];
			t.vertex[1].z = output_tex7_data[output_data_index + 2];

			t.vertex[2].x = output_tex8_data[output_data_index + 0];
			t.vertex[2].y = output_tex8_data[output_data_index + 1];
			t.vertex[2].z = output_tex8_data[output_data_index + 2];

			triangles.push_back(t);

			t.vertex[0].x = output_tex9_data[output_data_index + 0];
			t.vertex[0].y = output_tex9_data[output_data_index + 1];
			t.vertex[0].z = output_tex9_data[output_data_index + 2];

			t.vertex[1].x = output_tex10_data[output_data_index + 0];
			t.vertex[1].y = output_tex10_data[output_data_index + 1];
			t.vertex[1].z = output_tex10_data[output_data_index + 2];

			t.vertex[2].x = output_tex11_data[output_data_index + 0];
			t.vertex[2].y = output_tex11_data[output_data_index + 1];
			t.vertex[2].z = output_tex11_data[output_data_index + 2];

			triangles.push_back(t);

			t.vertex[0].x = output_tex12_data[output_data_index + 0];
			t.vertex[0].y = output_tex12_data[output_data_index + 1];
			t.vertex[0].z = output_tex12_data[output_data_index + 2];

			t.vertex[1].x = output_tex13_data[output_data_index + 0];
			t.vertex[1].y = output_tex13_data[output_data_index + 1];
			t.vertex[1].z = output_tex13_data[output_data_index + 2];

			t.vertex[2].x = output_tex14_data[output_data_index + 0];
			t.vertex[2].y = output_tex14_data[output_data_index + 1];
			t.vertex[2].z = output_tex14_data[output_data_index + 2];

			triangles.push_back(t);
		}
		else if (num_tris == 4)
		{

			triangle t;

			t.vertex[0].x = output_tex0_data[output_data_index + 0];
			t.vertex[0].y = output_tex0_data[output_data_index + 1];
			t.vertex[0].z = output_tex0_data[output_data_index + 2];

			t.vertex[1].x = output_tex1_data[output_data_index + 0];
			t.vertex[1].y = output_tex1_data[output_data_index + 1];
			t.vertex[1].z = output_tex1_data[output_data_index + 2];

			t.vertex[2].x = output_tex2_data[output_data_index + 0];
			t.vertex[2].y = output_tex2_data[output_data_index + 1];
			t.vertex[2].z = output_tex2_data[output_data_index + 2];

			triangles.push_back(t);

			t.vertex[0].x = output_tex3_data[output_data_index + 0];
			t.vertex[0].y = output_tex3_data[output_data_index + 1];
			t.vertex[0].z = output_tex3_data[output_data_index + 2];

			t.vertex[1].x = output_tex4_data[output_data_index + 0];
			t.vertex[1].y = output_tex4_data[output_data_index + 1];
			t.vertex[1].z = output_tex4_data[output_data_index + 2];

			t.vertex[2].x = output_tex5_data[output_data_index + 0];
			t.vertex[2].y = output_tex5_data[output_data_index + 1];
			t.vertex[2].z = output_tex5_data[output_data_index + 2];

			triangles.push_back(t);

			t.vertex[0].x = output_tex6_data[output_data_index + 0];
			t.vertex[0].y = output_tex6_data[output_data_index + 1];
			t.vertex[0].z = output_tex6_data[output_data_index + 2];

			t.vertex[1].x = output_tex7_data[output_data_index + 0];
			t.vertex[1].y = output_tex7_data[output_data_index + 1];
			t.vertex[1].z = output_tex7_data[output_data_index + 2];

			t.vertex[2].x = output_tex8_data[output_data_index + 0];
			t.vertex[2].y = output_tex8_data[output_data_index + 1];
			t.vertex[2].z = output_tex8_data[output_data_index + 2];

			triangles.push_back(t);

			t.vertex[0].x = output_tex9_data[output_data_index + 0];
			t.vertex[0].y = output_tex9_data[output_data_index + 1];
			t.vertex[0].z = output_tex9_data[output_data_index + 2];

			t.vertex[1].x = output_tex10_data[output_data_index + 0];
			t.vertex[1].y = output_tex10_data[output_data_index + 1];
			t.vertex[1].z = output_tex10_data[output_data_index + 2];

			t.vertex[2].x = output_tex11_data[output_data_index + 0];
			t.vertex[2].y = output_tex11_data[output_data_index + 1];
			t.vertex[2].z = output_tex11_data[output_data_index + 2];

			triangles.push_back(t);
		}
		else if (num_tris == 3)
		{
			triangle t;

			t.vertex[0].x = output_tex0_data[output_data_index + 0];
			t.vertex[0].y = output_tex0_data[output_data_index + 1];
			t.vertex[0].z = output_tex0_data[output_data_index + 2];

			t.vertex[1].x = output_tex1_data[output_data_index + 0];
			t.vertex[1].y = output_tex1_data[output_data_index + 1];
			t.vertex[1].z = output_tex1_data[output_data_index + 2];

			t.vertex[2].x = output_tex2_data[output_data_index + 0];
			t.vertex[2].y = output_tex2_data[output_data_index + 1];
			t.vertex[2].z = output_tex2_data[output_data_index + 2];

			triangles.push_back(t);

			t.vertex[0].x = output_tex3_data[output_data_index + 0];
			t.vertex[0].y = output_tex3_data[output_data_index + 1];
			t.vertex[0].z = output_tex3_data[output_data_index + 2];

			t.vertex[1].x = output_tex4_data[output_data_index + 0];
			t.vertex[1].y = output_tex4_data[output_data_index + 1];
			t.vertex[1].z = output_tex4_data[output_data_index + 2];

			t.vertex[2].x = output_tex5_data[output_data_index + 0];
			t.vertex[2].y = output_tex5_data[output_data_index + 1];
			t.vertex[2].z = output_tex5_data[output_data_index + 2];

			triangles.push_back(t);

			t.vertex[0].x = output_tex6_data[output_data_index + 0];
			t.vertex[0].y = output_tex6_data[output_data_index + 1];
			t.vertex[0].z = output_tex6_data[output_data_index + 2];

			t.vertex[1].x = output_tex7_data[output_data_index + 0];
			t.vertex[1].y = output_tex7_data[output_data_index + 1];
			t.vertex[1].z = output_tex7_data[output_data_index + 2];

			t.vertex[2].x = output_tex8_data[output_data_index + 0];
			t.vertex[2].y = output_tex8_data[output_data_index + 1];
			t.vertex[2].z = output_tex8_data[output_data_index + 2];

			triangles.push_back(t);


		}
		else if (num_tris == 2)
		{
			triangle t;

			t.vertex[0].x = output_tex0_data[output_data_index + 0];
			t.vertex[0].y = output_tex0_data[output_data_index + 1];
			t.vertex[0].z = output_tex0_data[output_data_index + 2];

			t.vertex[1].x = output_tex1_data[output_data_index + 0];
			t.vertex[1].y = output_tex1_data[output_data_index + 1];
			t.vertex[1].z = output_tex1_data[output_data_index + 2];

			t.vertex[2].x = output_tex2_data[output_data_index + 0];
			t.vertex[2].y = output_tex2_data[output_data_index + 1];
			t.vertex[2].z = output_tex2_data[output_data_index + 2];

			triangles.push_back(t);

			t.vertex[0].x = output_tex3_data[output_data_index + 0];
			t.vertex[0].y = output_tex3_data[output_data_index + 1];
			t.vertex[0].z = output_tex3_data[output_data_index + 2];

			t.vertex[1].x = output_tex4_data[output_data_index + 0];
			t.vertex[1].y = output_tex4_data[output_data_index + 1];
			t.vertex[1].z = output_tex4_data[output_data_index + 2];

			t.vertex[2].x = output_tex5_data[output_data_index + 0];
			t.vertex[2].y = output_tex5_data[output_data_index + 1];
			t.vertex[2].z = output_tex5_data[output_data_index + 2];

			triangles.push_back(t);
		}
		else if (num_tris == 1)
		{
			triangle t;

			t.vertex[0].x = output_tex0_data[output_data_index + 0];
			t.vertex[0].y = output_tex0_data[output_data_index + 1];
			t.vertex[0].z = output_tex0_data[output_data_index + 2];

			t.vertex[1].x = output_tex1_data[output_data_index + 0];
			t.vertex[1].y = output_tex1_data[output_data_index + 1];
			t.vertex[1].z = output_tex1_data[output_data_index + 2];

			t.vertex[2].x = output_tex2_data[output_data_index + 0];
			t.vertex[2].y = output_tex2_data[output_data_index + 1];
			t.vertex[2].z = output_tex2_data[output_data_index + 2];

			//cout << t.vertex[0].x << " " << t.vertex[0].y << " " << t.vertex[0].z << endl;
			//cout << t.vertex[1].x << " " << t.vertex[1].y << " " << t.vertex[1].z << endl;
			//cout << t.vertex[2].x << " " << t.vertex[2].y << " " << t.vertex[2].z << endl;
			//cout << endl;

			triangles.push_back(t);
		}
	}

	return true;
}
*/



	fractal_set_parameters fsp;

protected:

	ostringstream oss;
	vector<triangle> triangles;
	vector<vertex_3_with_normal> vertices_with_face_normals;

	void reclaim_all_but_vertex_buffer(void);
	void g0_draw_gpu(void);
	void g0_draw_cpu(void);

	int g0_stage_0_gpu(void);
	int g0_stage_1_gpu(void);
	int g0_stage_0_cpu(void);
	int g0_stage_1_cpu(void);

	int g1_stage_0(void);
	int g1_stage_1(void);
	int g1_stage_2(void);
	int g1_stage_3(void);
	int g1_stage_4(void);
	int g1_stage_5(void);

	int g2_stage_0(void);
	int g2_stage_1(void);

	int g3_stage_0_blue(void);
	int g3_stage_0_rainbow(void);

	float g0_step_size_x = 0, g0_step_size_y = 0, g0_step_size_z = 0;
	quaternion g0_Z;
	size_t g0_z;
	vector<float> g0_previous_slice;
	vector<float> g0_output_pixels, g0_input_pixels;
	vector<float> g0_xyplane0, g0_xyplane1;
	size_t g0_num_voxels;
	GLuint g0_compute_shader_program = 0;
	GLuint g0_tex_output = 0;
	GLuint g0_tex_input = 0;
	size_t g0_num_output_channels = 1;
	size_t g0_num_input_channels = 4;

	set<vertex_3_with_index> g1_vertex_set;
	vector<triangle>::const_iterator g1_i0;
	vector<vertex_3_with_index> g1_v;
	set<vertex_3_with_index>::const_iterator g1_i1;
	vector<vertex_3_with_index>::const_iterator g1_i2;
	set<vertex_3_with_index>::iterator g1_find_iter;
	vector<triangle>::iterator g1_i3;
	vector<triangle>::iterator g1_i4;
	size_t g1_i5;

	ofstream g2_out;
	vector<char> g2_buffer;
	size_t g2_num_triangles = 0;
	size_t g2_data_size = 0;
	size_t g2_buffer_size = 0, g2_bytes_remaining = 0, g2_bytes_written = 0;
	char* g2_cp = 0;
	vector<triangle>::const_iterator g2_i0;

	size_t g3_i0;

	js_state_machine_member_function_pointer ptr = 0;
	size_t g1_batch_size = 10000;
	size_t g2_i0_batch_size = 10000;
	size_t g2_i1_batch_size = 10 * 1024 * 1024;
	size_t g3_batch_size = 10000;
	size_t state = 0;

	quaternion_julia_set_equation_parser eqparser;
	logging_system* log_system;







};



#endif