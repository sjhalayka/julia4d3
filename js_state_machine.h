#ifndef JS_STATE_MACHINE_H
#define JS_STATE_MACHINE_H

#include "fractal_set_parameters.h"
#include "primitives.h"
#include "eqparse.h"
#include "logging_system.h"
#include "marching_cubes.h"
using namespace marching_cubes;



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



bool compile_and_link_compute_shader(const char* const file_name, GLuint& program, logging_system &ls);




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
	bool init(fractal_set_parameters& fsp_in, logging_system *ls);
	void cancel(void);
	void proceed(void);

	std::chrono::high_resolution_clock::time_point start_time, end_time;

protected:

	ostringstream oss;
	vector<triangle> triangles;
	vector<vertex_3_with_normal> vertices_with_face_normals;
	fractal_set_parameters fsp;

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
	size_t g2_i1_batch_size = 10*1024*1024;
	size_t g3_batch_size = 10000;
	size_t state = 0;

	quaternion_julia_set_equation_parser eqparser;
	logging_system* log_system;
};



#endif