#ifndef MESH_H
#define MESH_H

#include "primitives.h"





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


#include <iostream>
using std::cout;
using std::endl;

#include <fstream>
using std::ifstream;
using std::ofstream;

#include <ios>
using std::ios_base;
using std::ios;

#include <set>
using std::set;

#include <vector>
using std::vector;

#include <limits>
using std::numeric_limits;

#include <cstring> // for memcpy()


bool read_triangles_from_binary_stereo_lithography_file(vector<triangle> &triangles, const char *const file_name);
void add_box(vector<triangle>& triangles, float max_extent);
void scale_mesh(vector<triangle> &triangles, float max_extent);
void get_triangle_indices_and_vertices_with_face_normals_from_triangles(atomic_bool &stop_flag, mutex &m, vector<triangle> &t, vector<triangle_index> &triangle_indices, vector<vertex_3_with_normal> &vertices_with_face_normals);


#endif
