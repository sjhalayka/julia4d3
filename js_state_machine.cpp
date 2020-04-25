#include "js_state_machine.h"



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

size_t js_state_machine::get_state(void)
{
	return state;
}

js_state_machine::js_state_machine(void)
{
	ptr = 0;
	state = STATE_UNINITIALIZED;
	log_system = 0;
}

void js_state_machine::reclaim_all_but_vertex_buffer(void)
{
	g0_previous_slice.clear();
	g0_output_pixels.clear();
	g0_input_pixels.clear();
	g0_xyplane0.clear();
	g0_xyplane1.clear();
	g1_vertex_set.clear();
	g1_v.clear();
	g2_buffer.clear();

	triangles.clear();
	vertices_with_face_normals.clear();
}

bool js_state_machine::init(fractal_set_parameters& fsp_in, logging_system* ls)
{
	reclaim_all_but_vertex_buffer();
	vertex_data.clear();

	log_system = ls;
	fsp = fsp_in;

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

	string code = eqparser.emit_compute_shader_code(1, 1, fsp.max_iterations);

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

	return true;
}


void js_state_machine::cancel(void)
{
	ptr = 0;
	state = STATE_CANCELLED;
	reclaim_all_but_vertex_buffer();
}


void js_state_machine::proceed(void)
{
	if (ptr != 0)
		(this->*(this->ptr))();
}

void js_state_machine::g0_draw(void)
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
	glUniform1f(glGetUniformLocation(g0_compute_shader_program, "threshold"), fsp.infinity);

	// Run compute shader
	glDispatchCompute(fsp.resolution, fsp.resolution, 1);

	// Wait for compute shader to finish
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

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

	int js_state_machine::g0_stage_0(void)
	{
		g0_draw();

		g0_previous_slice = g0_output_pixels;

		g0_z++;
		g0_Z.z += g0_step_size_z;

		ptr = &js_state_machine::g0_stage_1;
		state = STATE_G0_STAGE_1;

		return 1;
	}

	int js_state_machine::g0_stage_1(void)
	{
		g0_draw();

		size_t box_count = 0;

		oss.clear();
		oss.str("");
		oss << "Calculating triangles from xy-plane pair " << g0_z << " of " << fsp.resolution - 1;

		if(0 != log_system)
		log_system->add_string_to_contents(oss.str());

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

			oss.clear();
			oss.str("");
			oss << "Welding vertices";

			if(0 != log_system)
				log_system->add_string_to_contents(oss.str());
		}
		else
		{
			g0_z++;
			g0_Z.z += g0_step_size_z;
			g0_previous_slice = g0_output_pixels;
		}

		return 1;
	}

	int js_state_machine::g1_stage_0(void)
	{
		size_t count = 0;

		for (; g1_i0 != triangles.end(); g1_i0++)
		{
			if (count == g1_batch_size)
				return 0;
			else
				count++;

			g1_vertex_set.insert(g1_i0->vertex[0]);
			g1_vertex_set.insert(g1_i0->vertex[1]);
			g1_vertex_set.insert(g1_i0->vertex[2]);
		}

		ptr = &js_state_machine::g1_stage_1;
		state = STATE_G1_STAGE_1;
		g1_i1 = g1_vertex_set.begin();

		oss.clear();
		oss.str("");
		oss << "Generating indices";

		if (0 != log_system)
			log_system->add_string_to_contents(oss.str());

		return 1;
	}

	int js_state_machine::g1_stage_1(void)
	{
		size_t count = 0;

		for (; g1_i1 != g1_vertex_set.end(); g1_i1++)
		{
			if (count == g1_batch_size)
				return 0;
			else
				count++;

			size_t index = g1_v.size();
			g1_v.push_back(*g1_i1);
			g1_v[index].index = static_cast<GLuint>(index);
		}

		g1_vertex_set.clear();

		ptr = &js_state_machine::g1_stage_2;
		state = STATE_G1_STAGE_2;
		g1_i2 = g1_v.begin();

		oss.clear();
		oss.str("");
		oss << "Generating vertex set";
		if (0 != log_system)
			log_system->add_string_to_contents(oss.str());

		return 1;
	}

	int js_state_machine::g1_stage_2(void)
	{
		size_t count = 0;

		for (; g1_i2 != g1_v.end(); g1_i2++)
		{
			if (count == g1_batch_size)
				return 0;
			else
				count++;

			g1_vertex_set.insert(*g1_i2);
		}


		oss.clear();
		oss.str("");
		oss << "Assigning vertex indices";
		if (0 != log_system)
			log_system->add_string_to_contents(oss.str());

		ptr = &js_state_machine::g1_stage_3;
		state = STATE_G1_STAGE_3;
		g1_i3 = triangles.begin();

		return 1;
	}

	int js_state_machine::g1_stage_3(void)
	{
		size_t count = 0;

		for (; g1_i3 != triangles.end(); g1_i3++)
		{
			if (count == g1_batch_size)
				return 0;
			else
				count++;

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

		oss.clear();
		oss.str("");
		oss << "Generating normal data";
		if (0 != log_system)
			log_system->add_string_to_contents(oss.str());

		return 1;
	}

	int js_state_machine::g1_stage_4(void)
	{
		size_t count = 0;

		for (; g1_i4 != triangles.end(); g1_i4++)
		{
			if (count == g1_batch_size)
				return 0;
			else
				count++;

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

		oss.clear();
		oss.str("");
		oss << "Assigning normals";
		if (0 != log_system)
			log_system->add_string_to_contents(oss.str());

		return 1;
	}

	int js_state_machine::g1_stage_5(void)
	{
		size_t count = 0;

		for (; g1_i5 < g1_v.size(); g1_i5++)
		{
			if (count == g1_batch_size)
				return 0;
			else
				count++;

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

		ptr = &js_state_machine::g2_stage_0;
		state = STATE_G1_STAGE_0;

		if (0 == triangles.size())
		{
			ptr = 0;
			state = STATE_FINISHED;
			reclaim_all_but_vertex_buffer();
			return 0;
		}

		// Write to file.
		g2_out.open("out.stl", ios_base::binary);

		if (g2_out.fail())
		{
			ptr = 0;
			state = STATE_UNINITIALIZED;
			reclaim_all_but_vertex_buffer();
			return 0;
		}

		const size_t header_size = 80;
		g2_buffer.resize(header_size, 0);
		g2_num_triangles = static_cast<unsigned int>(triangles.size()); // Must be 4-byte unsigned int.

		g2_out.write(reinterpret_cast<const char*>(&(g2_buffer[0])), header_size);
		g2_out.write(reinterpret_cast<const char*>(&g2_num_triangles), sizeof(unsigned int));

		g2_data_size = (12 * sizeof(float) + sizeof(short unsigned int)) * g2_num_triangles;
		g2_buffer.resize(g2_data_size, 0);

		g2_cp = &g2_buffer[0];
		g2_i0 = triangles.begin();

		oss.clear();
		oss.str("");
		oss << "Building file buffer in memory";
		if (0 != log_system)
			log_system->add_string_to_contents(oss.str());

		return 1;
	}

	int js_state_machine::g2_stage_0(void)
	{
		size_t count = 0;

		for (; g2_i0 != triangles.end(); g2_i0++)
		{
			if (count == g2_i0_batch_size)
				return 0;
			else
				count++;

			// Get face normal.
			vertex_3 v0 = g2_i0->vertex[1] - g2_i0->vertex[0];
			vertex_3 v1 = g2_i0->vertex[2] - g2_i0->vertex[0];
			vertex_3 normal = v0.cross(v1);
			normal.normalize();

			memcpy(g2_cp, &normal.x, sizeof(float)); g2_cp += sizeof(float);
			memcpy(g2_cp, &normal.y, sizeof(float)); g2_cp += sizeof(float);
			memcpy(g2_cp, &normal.z, sizeof(float)); g2_cp += sizeof(float);

			memcpy(g2_cp, &g2_i0->vertex[0].x, sizeof(float)); g2_cp += sizeof(float);
			memcpy(g2_cp, &g2_i0->vertex[0].y, sizeof(float)); g2_cp += sizeof(float);
			memcpy(g2_cp, &g2_i0->vertex[0].z, sizeof(float)); g2_cp += sizeof(float);
			memcpy(g2_cp, &g2_i0->vertex[1].x, sizeof(float)); g2_cp += sizeof(float);
			memcpy(g2_cp, &g2_i0->vertex[1].y, sizeof(float)); g2_cp += sizeof(float);
			memcpy(g2_cp, &g2_i0->vertex[1].z, sizeof(float)); g2_cp += sizeof(float);
			memcpy(g2_cp, &g2_i0->vertex[2].x, sizeof(float)); g2_cp += sizeof(float);
			memcpy(g2_cp, &g2_i0->vertex[2].y, sizeof(float)); g2_cp += sizeof(float);
			memcpy(g2_cp, &g2_i0->vertex[2].z, sizeof(float)); g2_cp += sizeof(float);

			g2_cp += sizeof(short unsigned int);
		}

		ptr = &js_state_machine::g2_stage_1;
		state = STATE_G2_STAGE_1;

		// init g2_stage_1
		g2_bytes_remaining = g2_data_size;

		oss.clear();
		oss.str("");
		oss << "Writing buffer to file \"out.stl\"";
		if (0 != log_system)
			log_system->add_string_to_contents(oss.str());

		return 1;
	}

	int js_state_machine::g2_stage_1(void)
	{
		size_t bytes_to_write = 0;

		if (g2_i1_batch_size > g2_bytes_remaining)
			bytes_to_write = g2_bytes_remaining;
		else
			bytes_to_write = g2_i1_batch_size;

		g2_out.write(reinterpret_cast<const char*>(&g2_buffer[g2_bytes_written]), bytes_to_write);
		g2_bytes_remaining -= bytes_to_write;

		if (g2_bytes_remaining == 0)
		{
			g2_out.close();

			g3_i0 = 0;
			ptr = &js_state_machine::g3_stage_0;
			state = STATE_G3_STAGE_0;

			oss.clear();
			oss.str("");
			oss << "Building OpenGL vertex buffer";

			if (0 != log_system)
				log_system->add_string_to_contents(oss.str());
		}

		return 1;
	}

	int js_state_machine::g3_stage_0(void)
	{
		size_t count = 0;

		for (; g3_i0 != triangles.size(); g3_i0++)
		{
			if (count == g3_batch_size)
				return 0;
			else
				count++;

			vertex_3 colour(0.0f, 0.8f, 1.0f);

			size_t v0_index = triangles[g3_i0].vertex[0].index;
			size_t v1_index = triangles[g3_i0].vertex[1].index;
			size_t v2_index = triangles[g3_i0].vertex[2].index;

			vertex_3 v0_fn(vertices_with_face_normals[v0_index].nx, vertices_with_face_normals[v0_index].ny, vertices_with_face_normals[v0_index].nz);
			vertex_3 v1_fn(vertices_with_face_normals[v1_index].nx, vertices_with_face_normals[v1_index].ny, vertices_with_face_normals[v1_index].nz);
			vertex_3 v2_fn(vertices_with_face_normals[v2_index].nx, vertices_with_face_normals[v2_index].ny, vertices_with_face_normals[v2_index].nz);

			vertex_3 v0(triangles[g3_i0].vertex[0].x, triangles[g3_i0].vertex[0].y, triangles[g3_i0].vertex[0].z);
			vertex_3 v1(triangles[g3_i0].vertex[1].x, triangles[g3_i0].vertex[1].y, triangles[g3_i0].vertex[1].z);
			vertex_3 v2(triangles[g3_i0].vertex[2].x, triangles[g3_i0].vertex[2].y, triangles[g3_i0].vertex[2].z);

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

		ptr = 0;
		state = STATE_FINISHED;
		reclaim_all_but_vertex_buffer();
		return 0;
	}