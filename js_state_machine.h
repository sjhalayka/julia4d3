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
		glUseProgram(g0_mc_shader.get_program());
		glUniform1f(glGetUniformLocation(g0_mc_shader.get_program(), "threshold"), fsp.infinity);

		// Make room for a large number of grid_cubes data. 8 corners, 4 floats per corner
		vector<float> points_vertex_data;

		points_vertex_data.resize((x_res - 1) * (y_res - 1) * 8 * 4);

		const float x_step_size = (x_grid_max - x_grid_min) / (x_res - 1);
		const float y_step_size = (y_grid_max - y_grid_min) / (y_res - 1);
		const float z_step_size = (z_grid_max - z_grid_min) / (z_res - 1);

		size_t index = 0;

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

				points_vertex_data[index++] = temp_cube.vertex[0].x;
				points_vertex_data[index++] = temp_cube.vertex[0].y;
				points_vertex_data[index++] = temp_cube.vertex[0].z;
				points_vertex_data[index++] = temp_cube.value[0];

				points_vertex_data[index++] = temp_cube.vertex[1].x;
				points_vertex_data[index++] = temp_cube.vertex[1].y;
				points_vertex_data[index++] = temp_cube.vertex[1].z;
				points_vertex_data[index++] = temp_cube.value[1];

				points_vertex_data[index++] = temp_cube.vertex[2].x;
				points_vertex_data[index++] = temp_cube.vertex[2].y;
				points_vertex_data[index++] = temp_cube.vertex[2].z;
				points_vertex_data[index++] = temp_cube.value[2];

				points_vertex_data[index++] = temp_cube.vertex[3].x;
				points_vertex_data[index++] = temp_cube.vertex[3].y;
				points_vertex_data[index++] = temp_cube.vertex[3].z;
				points_vertex_data[index++] = temp_cube.value[3];

				points_vertex_data[index++] = temp_cube.vertex[4].x;
				points_vertex_data[index++] = temp_cube.vertex[4].y;
				points_vertex_data[index++] = temp_cube.vertex[4].z;
				points_vertex_data[index++] = temp_cube.value[4];

				points_vertex_data[index++] = temp_cube.vertex[5].x;
				points_vertex_data[index++] = temp_cube.vertex[5].y;
				points_vertex_data[index++] = temp_cube.vertex[5].z;
				points_vertex_data[index++] = temp_cube.value[5];

				points_vertex_data[index++] = temp_cube.vertex[6].x;
				points_vertex_data[index++] = temp_cube.vertex[6].y;
				points_vertex_data[index++] = temp_cube.vertex[6].z;
				points_vertex_data[index++] = temp_cube.value[6];

				points_vertex_data[index++] = temp_cube.vertex[7].x;
				points_vertex_data[index++] = temp_cube.vertex[7].y;
				points_vertex_data[index++] = temp_cube.vertex[7].z;
				points_vertex_data[index++] = temp_cube.value[7];
			}
		}



//	https://github.com/progschj/OpenGL-Examples/blob/master/09transform_feedback.cpp

		const GLuint components_per_position = 4;
		const GLuint components_per_vertex = 8 * components_per_position;

		static GLuint point_buffer;

		glDeleteBuffers(1, &point_buffer);
		glGenBuffers(1, &point_buffer);

		const GLuint num_vertices = static_cast<GLuint>(points_vertex_data.size()) / components_per_vertex;

		glBindBuffer(GL_ARRAY_BUFFER, point_buffer);
		glBufferData(GL_ARRAY_BUFFER, points_vertex_data.size() * sizeof(GLfloat), &points_vertex_data[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(glGetAttribLocation(g0_mc_shader.get_program(), "position0"));
		glVertexAttribPointer(glGetAttribLocation(g0_mc_shader.get_program(), "position0"),
			components_per_position,
			GL_FLOAT,
			GL_FALSE,
			components_per_vertex * sizeof(GLfloat),
			0);

		glEnableVertexAttribArray(glGetAttribLocation(g0_mc_shader.get_program(), "position1"));
		glVertexAttribPointer(glGetAttribLocation(g0_mc_shader.get_program(), "position1"),
			components_per_position,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(1 * components_per_position * sizeof(GLfloat)));

		glEnableVertexAttribArray(glGetAttribLocation(g0_mc_shader.get_program(), "position2"));
		glVertexAttribPointer(glGetAttribLocation(g0_mc_shader.get_program(), "position2"),
			components_per_position,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(2 * components_per_position * sizeof(GLfloat)));

		glEnableVertexAttribArray(glGetAttribLocation(g0_mc_shader.get_program(), "position3"));
		glVertexAttribPointer(glGetAttribLocation(g0_mc_shader.get_program(), "position3"),
			components_per_position,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(3 * components_per_position * sizeof(GLfloat)));

		glEnableVertexAttribArray(glGetAttribLocation(g0_mc_shader.get_program(), "position4"));
		glVertexAttribPointer(glGetAttribLocation(g0_mc_shader.get_program(), "position4"),
			components_per_position,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(4 * components_per_position * sizeof(GLfloat)));

		glEnableVertexAttribArray(glGetAttribLocation(g0_mc_shader.get_program(), "position5"));
		glVertexAttribPointer(glGetAttribLocation(g0_mc_shader.get_program(), "position5"),
			components_per_position,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(5 * components_per_position * sizeof(GLfloat)));

		glEnableVertexAttribArray(glGetAttribLocation(g0_mc_shader.get_program(), "position6"));
		glVertexAttribPointer(glGetAttribLocation(g0_mc_shader.get_program(), "position6"),
			components_per_position,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(6 * components_per_position * sizeof(GLfloat)));

		glEnableVertexAttribArray(glGetAttribLocation(g0_mc_shader.get_program(), "position7"));
		glVertexAttribPointer(glGetAttribLocation(g0_mc_shader.get_program(), "position7"),
			components_per_position,	
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(7 * components_per_position * sizeof(GLfloat)));
		


		size_t max_triangles_per_geometry_shader = 5;
		size_t num_vertices_per_triangle = 3;
		size_t num_floats_per_vertex = 3;

		GLuint tbo;
		glGenBuffers(1, &tbo);
		glBindBuffer(GL_ARRAY_BUFFER, tbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *num_vertices* max_triangles_per_geometry_shader*num_vertices_per_triangle* num_floats_per_vertex, nullptr, GL_STATIC_READ);

		GLuint query;
		glGenQueries(1, &query);

		// Perform feedback transform
		glEnable(GL_RASTERIZER_DISCARD);

		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);

		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
		glBeginTransformFeedback(GL_TRIANGLES);
		glDrawArrays(GL_POINTS, 0, num_vertices);
		glEndTransformFeedback();
		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

		glDisable(GL_RASTERIZER_DISCARD);

		glFlush();

		GLuint primitives;
		glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);

		vector<GLfloat> feedback(num_vertices* max_triangles_per_geometry_shader* num_vertices_per_triangle* num_floats_per_vertex);
		glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(GLfloat)* feedback.size(), &feedback[0]);

		glDeleteQueries(1, &query);
		glDeleteBuffers(1, &tbo);

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




		/*


		GLuint query;

		glGenQueries(1, &query);

		glBeginQuery(GL_PRIMITIVES_GENERATED, query);

		vector<float> in_data((fsp.resolution - 1)* (fsp.resolution - 1) * 5 * 9);

		GLuint tbo;
		glGenBuffers(1, &tbo);
		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tbo);
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, in_data.size() * sizeof(float), &in_data[0], GL_STATIC_READ);

		glBeginTransformFeedback(GL_TRIANGLES);
		glDrawArrays(GL_POINTS, 0, num_vertices);
		glEndTransformFeedback();

		glEndQuery(GL_PRIMITIVES_GENERATED);

		GLuint primitives;
		glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);


		cout << primitives << endl;

		vector<float> feedback(in_data.size(), 0.0f);
		glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, feedback.size() * sizeof(float), &feedback[0]);

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



		return true;
	}

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
	vertex_geometry_shader g0_mc_shader;

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