#ifndef JS_STATE_MACHINE_H
#define JS_STATE_MACHINE_H


#include "fractal_set_parameters.h"
#include "primitives.h"
#include "eqparse.h"

#include <vector>
using std::vector;
#include <fstream>
using std::ifstream;

#include<string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include <mutex>
using std::mutex;





bool compile_and_link_compute_shader(const char* const file_name, GLuint& program)
{
	// Read in compute shader contents
	ifstream infile(file_name);

	if (infile.fail())
	{
		cout << "Could not open compute shader source file " << file_name << endl;
		return false;
	}

	string shader_code;
	string line;

	while (getline(infile, line))
	{
		shader_code += line;
		shader_code += "\n";
	}

	// Compile compute shader
	const char* cch = 0;
	GLint status = GL_FALSE;

	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);

	glShaderSource(shader, 1, &(cch = shader_code.c_str()), NULL);

	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (GL_FALSE == status)
	{
		string status_string = "Compute shader compile error.\n";
		vector<GLchar> buf(4096, '\0');
		glGetShaderInfoLog(shader, 4095, 0, &buf[0]);

		for (size_t i = 0; i < buf.size(); i++)
			if ('\0' != buf[i])
				status_string += buf[i];

		status_string += '\n';

		cout << status_string << endl;

		glDeleteShader(shader);

		return false;
	}

	// Link compute shader
	program = glCreateProgram();
	glAttachShader(program, shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (GL_FALSE == status)
	{
		string status_string = "Program link error.\n";
		vector<GLchar> buf(4096, '\0');
		glGetShaderInfoLog(program, 4095, 0, &buf[0]);

		for (size_t i = 0; i < buf.size(); i++)
			if ('\0' != buf[i])
				status_string += buf[i];

		status_string += '\n';

		cout << status_string << endl;

		glDetachShader(program, shader);
		glDeleteShader(shader);
		glDeleteProgram(program);

		return false;
	}

	// The shader is no longer needed now that the program
	// has been linked
	glDetachShader(program, shader);
	glDeleteShader(shader);

	return true;
}




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







class js_state_machine
{
public:

	vector<triangle> triangles;
	vector<vertex_3_with_normal> vertices_with_face_normals;

	size_t get_state(void)
	{
		return state;
	}

	js_state_machine(void)
	{
		ptr = 0;
		state = STATE_UNINITIALIZED;
	}

	bool init(fractal_set_parameters fsp_in)
	{
		cout << "Entering jsm init" << endl;
		triangles.clear();

		fsp = fsp_in;

		cout << "fsp resolution " << fsp.resolution << endl;

		string error_string;
		quaternion C;
		C.x = fsp.C_x;
		C.y = fsp.C_y;
		C.z = fsp.C_z;
		C.w = fsp.C_w;

		if (glIsProgram(g0_compute_shader_program))
			glDeleteProgram(g0_compute_shader_program);

		if (glIsTexture(g0_tex_output))
			glDeleteTextures(1, &g0_tex_output);

		if (glIsTexture(g0_tex_input))
			glDeleteTextures(1, &g0_tex_input);


		if (false == eqparser.setup(fsp.equation_text, error_string, C))
		{
			cout << "eqparser setup error: " << error_string << endl;
			return false;
		}

		string code = eqparser.emit_compute_shader_code(fsp.resolution, fsp.resolution);

		ofstream of("julia.cs.glsl");
		of << code;
		of.close();

		if (false == compile_and_link_compute_shader("julia.cs.glsl", g0_compute_shader_program))
		{
			cout << "Compute shader compile error" << endl;
			return false;
		}

		glGenTextures(1, &g0_tex_output);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g0_tex_output);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, fsp.resolution, fsp.resolution, 0, GL_RED, GL_FLOAT, NULL);
		glBindImageTexture(0, g0_tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

		// Generate input texture
		glGenTextures(1, &g0_tex_input);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, g0_tex_input);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		g0_num_output_channels = 1;
		g0_output_pixels.resize(fsp.resolution * fsp.resolution * g0_num_output_channels);
		g0_num_input_channels = 4;
		g0_input_pixels.resize(fsp.resolution * fsp.resolution * g0_num_input_channels);

		// We must keep track of both the current and the previous slices, 
		// so that they can be used as input for the Marching Cubes algorithm
		g0_previous_slice = g0_output_pixels;

		g0_num_voxels = fsp.resolution * fsp.resolution;

		g0_xyplane0.resize(g0_num_voxels, 0);
		g0_xyplane1.resize(g0_num_voxels, 0);

		g0_step_size_x = (fsp.x_max - fsp.x_min) / static_cast<float>(fsp.resolution - 1);
		g0_step_size_y = (fsp.y_max - fsp.y_min) / static_cast<float>(fsp.resolution - 1);
		g0_step_size_z = (fsp.z_max - fsp.z_min) / static_cast<float>(fsp.resolution - 1);

		g0_z = 0;

		g0_Z.x = fsp.x_min;
		g0_Z.y = fsp.y_min;
		g0_Z.z = fsp.z_min;
		g0_Z.w = fsp.Z_w;

		ptr = &js_state_machine::g0_stage_0;
		state = STATE_G0_STAGE_0;

		cout << "jsm init exit" << endl;
		// init stage 0 variables
		// set func ptr to g0_stage_0
		// set state to STATE_G0_STAGE_0
		return true;
	}

	void proceed(void)
	{
		cout << ptr << endl;

		if (ptr != 0)
			(this->*(this->ptr))();
	}

	void g0_draw(void)
	{
		g0_Z.x = fsp.x_min;

		// Create pixel array to be used as input for the compute shader
		for (size_t x = 0; x < fsp.resolution; x++, g0_Z.x += g0_step_size_x)
		{
			g0_Z.y = fsp.y_min;

			for (size_t y = 0; y < fsp.resolution; y++, g0_Z.y += g0_step_size_y)
			{
				const size_t index = g0_num_input_channels * (x * fsp.resolution + y);

				g0_input_pixels[index + 0] = g0_Z.x;
				g0_input_pixels[index + 1] = g0_Z.y;
				g0_input_pixels[index + 2] = g0_Z.z;
				g0_input_pixels[index + 3] = g0_Z.w;
			}
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g0_tex_output);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, fsp.resolution, fsp.resolution, 0, GL_RED, GL_FLOAT, NULL);
		glBindImageTexture(0, g0_tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, g0_tex_input);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Run the compute shader
		glActiveTexture(GL_TEXTURE1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, fsp.resolution, fsp.resolution, 0, GL_RGBA, GL_FLOAT, &g0_input_pixels[0]);
		glBindImageTexture(1, g0_tex_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

		glUseProgram(g0_compute_shader_program);

		// Pass in the input image and quaternion Julia set parameters as uniforms
		glUniform1i(glGetUniformLocation(g0_compute_shader_program, "input_image"), 1); // use GL_TEXTURE1
		glUniform4f(glGetUniformLocation(g0_compute_shader_program, "c"), fsp.C_x, fsp.C_y, fsp.C_z, fsp.C_w);
		glUniform1i(glGetUniformLocation(g0_compute_shader_program, "max_iterations"), fsp.max_iterations);
		glUniform1f(glGetUniformLocation(g0_compute_shader_program, "threshold"), fsp.infinity);

		cout << "dispatch compute" << endl;

		// Run compute shader
		glDispatchCompute(fsp.resolution, fsp.resolution, 1);

		// Wait for compute shader to finish
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		cout << "copy to texture" << endl;

		// Copy output pixel array to CPU as texture 0
		glActiveTexture(GL_TEXTURE0);
		glBindImageTexture(0, g0_tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &g0_output_pixels[0]);

		quaternion tempq;
		tempq.x = fsp.x_min;
		tempq.y = fsp.y_min;
		tempq.z = g0_Z.z;
		tempq.w = 1.0f;


		// Make a border, so that the mesh is closed around the edges
		for (size_t x = 0; x < fsp.resolution; x++, tempq.x += g0_step_size_x)
		{
			tempq.y = fsp.y_min;

			for (size_t y = 0; y < fsp.resolution; y++, tempq.y += g0_step_size_y)
			{

				if (g0_z == 0 || g0_z == fsp.resolution - 1 ||
					x == 0 || x == fsp.resolution - 1 ||
					y == 0 || y == fsp.resolution - 1)
				{
					const size_t index = g0_num_output_channels * (y * fsp.resolution + x);
					g0_output_pixels[index] = fsp.infinity + 1.0f;
				}
				else
				{
					const float y_span = (fsp.y_max - fsp.y_min);
					const float curr_span = 1.0f - static_cast<float>(fsp.y_max - tempq.y) / y_span;
					const size_t index = g0_num_output_channels * (x * fsp.resolution + y);

					if (fsp.use_pedestal == true && curr_span >= fsp.pedestal_y_start && curr_span <= fsp.pedestal_y_end)
					{
						g0_output_pixels[index] = fsp.infinity - 0.00001f;
					}
				}
			}
		}
	}

protected:
	int g0_stage_0(void)
	{
		cout << "Enter g0_stage_0" << endl;

		g0_draw();

		g0_previous_slice = g0_output_pixels;

		g0_z++;
		g0_Z.z += g0_step_size_z;

		ptr = &js_state_machine::g0_stage_1;
		state = STATE_G0_STAGE_1;

		cout << "exit g0_stage_0" << endl;

		return 1;
	}

	int g0_stage_1(void)
	{
		cout << "Enter g0_stage_1" << endl;

		g0_draw();

		size_t box_count = 0;

		ostringstream oss;

		oss.clear();
		oss.str("");
		oss << "Calculating triangles from xy-plane pair " << g0_z << " of " << fsp.resolution - 1;

		cout << oss.str() << endl;

		// Calculate triangles for the xy-planes corresponding to z - 1 and z by marching cubes.
		tesselate_adjacent_xy_plane_pair(
			box_count,
			g0_previous_slice, g0_output_pixels,
			g0_z - 1,
			triangles,
			fsp.infinity, // Use threshold as isovalue.
			fsp.x_min, fsp.x_max, fsp.resolution,
			fsp.y_min, fsp.y_max, fsp.resolution,
			fsp.z_min, fsp.z_max, fsp.resolution);

		if (g0_z >= fsp.resolution - 1)
		{
			ptr = &js_state_machine::g1_stage_0;
			state = STATE_G1_STAGE_0;
			g1_i0 = triangles.begin();
			cout << "done stage g0_stage1" << endl;
		}
		else
		{
			cout << "continuing stage g0_stage1" << endl;
			g0_z++;
			g0_Z.z += g0_step_size_z;
			g0_previous_slice = g0_output_pixels;
		}

		cout << "exit g0_stage_1" << endl;

		return 1;
	}

	int g1_stage_0(void)
	{
		size_t count = 0;

		for (; g1_i0 != triangles.end(); g1_i0++)
		{
			if (count == g1_batch_size)
				return 0;

			g1_vertex_set.insert(g1_i0->vertex[0]);
			g1_vertex_set.insert(g1_i0->vertex[1]);
			g1_vertex_set.insert(g1_i0->vertex[2]);
		}

		ptr = &js_state_machine::g1_stage_1;
		state = STATE_G1_STAGE_1;
		g1_i1 = g1_vertex_set.begin();

		return 1;
	}

	int g1_stage_1(void)
	{
		size_t count = 0;

		for (; g1_i1 != g1_vertex_set.end(); g1_i1++)
		{
			if (count == g1_batch_size)
				return 0;

			size_t index = g1_v.size();
			g1_v.push_back(*g1_i1);
			g1_v[index].index = static_cast<GLuint>(index);
		}

		g1_vertex_set.clear();

		ptr = &js_state_machine::g1_stage_2;
		state = STATE_G1_STAGE_2;
		g1_i2 = g1_v.begin();

		return 1;
	}

	int g1_stage_2(void)
	{
		size_t count = 0;

		for (; g1_i2 != g1_v.end(); g1_i2++)
		{
			if (count == g1_batch_size)
				return 0;

			g1_vertex_set.insert(*g1_i2);
		}

		ptr = &js_state_machine::g1_stage_3;
		state = STATE_G1_STAGE_3;
		g1_i3 = triangles.begin();

		return 1;
	}

	int g1_stage_3(void)
	{
		size_t count = 0;

		for (; g1_i3 != triangles.end(); g1_i3++)
		{
			if (count == g1_batch_size)
				return 0;

			g1_find_iter = g1_vertex_set.find(g1_i3->vertex[0]);
			g1_i3->vertex[0].index = g1_find_iter->index;

			g1_find_iter = g1_vertex_set.find(g1_i3->vertex[1]);
			g1_i3->vertex[1].index = g1_find_iter->index;

			g1_find_iter = g1_vertex_set.find(g1_i3->vertex[2]);
			g1_i3->vertex[2].index = g1_find_iter->index;
		}

		g1_vertex_set.clear();
		vertices_with_face_normals.resize(g1_v.size());

		ptr = &js_state_machine::g1_stage_4;
		state = STATE_G1_STAGE_4;
		g1_i4 = triangles.begin();

		return 1;
	}

	int g1_stage_4(void)
	{
		size_t count = 0;

		for (; g1_i4 != triangles.end(); g1_i4++)
		{
			if (count == g1_batch_size)
				return 0;

			vertex_3 v0 = g1_i4->vertex[1] - g1_i4->vertex[0];
			vertex_3 v1 = g1_i4->vertex[2] - g1_i4->vertex[0];
			vertex_3 fn = v0.cross(v1);
			fn.normalize();

			vertices_with_face_normals[g1_i4->vertex[0].index].nx += fn.x;
			vertices_with_face_normals[g1_i4->vertex[0].index].ny += fn.y;
			vertices_with_face_normals[g1_i4->vertex[0].index].nz += fn.z;
			vertices_with_face_normals[g1_i4->vertex[1].index].nx += fn.x;
			vertices_with_face_normals[g1_i4->vertex[1].index].ny += fn.y;
			vertices_with_face_normals[g1_i4->vertex[1].index].nz += fn.z;
			vertices_with_face_normals[g1_i4->vertex[2].index].nx += fn.x;
			vertices_with_face_normals[g1_i4->vertex[2].index].ny += fn.y;
			vertices_with_face_normals[g1_i4->vertex[2].index].nz += fn.z;
		}

		ptr = &js_state_machine::g1_stage_5;
		state = STATE_G1_STAGE_5;
		g1_i5 = 0;

		return 1;
	}

	int g1_stage_5(void)
	{
		size_t count = 0;

		for (; g1_i5 < g1_v.size(); g1_i5++)
		{
			if (count == g1_batch_size)
				return 0;

			// Assign vertex spatial comoponents
			vertices_with_face_normals[g1_i5].x = g1_v[g1_i5].x;
			vertices_with_face_normals[g1_i5].y = g1_v[g1_i5].y;
			vertices_with_face_normals[g1_i5].z = g1_v[g1_i5].z;

			// Normalize face normal
			vertex_3 temp_face_normal(vertices_with_face_normals[g1_i5].nx, vertices_with_face_normals[g1_i5].ny, vertices_with_face_normals[g1_i5].nz);
			temp_face_normal.normalize();

			vertices_with_face_normals[g1_i5].nx = temp_face_normal.x;
			vertices_with_face_normals[g1_i5].ny = temp_face_normal.y;
			vertices_with_face_normals[g1_i5].nz = temp_face_normal.z;
		}

		ptr = 0;
		state = STATE_FINISHED;

		return 1;
	}



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
	size_t g2_buffer_size = 0, g2_bytes_written = 0;
	char* g2_cp = 0;
	vector<triangle>::const_iterator g2_i0;




	fractal_set_parameters fsp;
	js_state_machine_member_function_pointer ptr = 0;
	size_t g1_batch_size = 10000;
	size_t g2_batch_size = 10*1048576;
	size_t state = 0;



	quaternion_julia_set_equation_parser eqparser;
};



#endif