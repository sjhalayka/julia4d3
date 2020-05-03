// Modified from Paul Bourke, Polygonising a Scalar Field
// Source code by Shawn Halayka
// Source code is in the public domain


#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H


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
using std::ofstream;

#include <iomanip>
using std::ios_base;

#include <vector>
using std::vector;

#include <set>
using std::set;




namespace marching_cubes
{
	class grid_cube
	{
	public:
		vertex_3 vertex[8];
		float value[8];
	};

	vertex_3 vertex_interp(const float isovalue, vertex_3 p1, vertex_3 p2, float valp1, float valp2);
	short unsigned int tesselate_grid_cube(const float isovalue, const grid_cube &grid, triangle *const triangles);
};

#endif
