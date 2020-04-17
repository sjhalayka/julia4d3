#include "mesh.h"








bool read_triangles_from_binary_stereo_lithography_file(vector<triangle> &triangles, const char *const file_name)
{
	triangles.clear();
	
    // Write to file.
    ifstream in(file_name, ios_base::binary);
 
    if(in.fail())
        return false;

	const size_t header_size = 80;
	vector<char> buffer(header_size, 0);
	unsigned int num_triangles = 0; // Must be 4-byte unsigned int.
	vertex_3 normal;

	// Read header.
	in.read(reinterpret_cast<char *>(&(buffer[0])), header_size);
	
	if(header_size != in.gcount())
		return false;

	if( 's' == tolower(buffer[0]) &&
		'o' == tolower(buffer[1]) && 
		'l' == tolower(buffer[2]) && 
		'i' == tolower(buffer[3]) && 
		'd' == tolower(buffer[4]) )
	{
		cout << "Encountered ASCII STL file header -- aborting." << endl;
		return false;
	}

	// Read number of triangles.
	in.read(reinterpret_cast<char *>(&num_triangles), sizeof(unsigned int));
	
	if(sizeof(unsigned int) != in.gcount())
		return false;

	triangles.resize(num_triangles);

	// Enough bytes for twelve 4-byte floats plus one 2-byte integer, per triangle.
	const size_t data_size = (12*sizeof(float) + sizeof(short unsigned int)) * num_triangles;
	buffer.resize(data_size, 0);

	in.read(reinterpret_cast<char *>(&buffer[0]), data_size);

	if(data_size != in.gcount())
		return false;

	// Use a pointer to assist with the copying.
	// Should probably use std::copy() instead, but memcpy() does the trick, so whatever...
	char *cp = &buffer[0];

    for(vector<triangle>::iterator i = triangles.begin(); i != triangles.end(); i++)
    {
		// Skip face normal.
		cp += 3*sizeof(float);
		
		// Get vertices.
		memcpy(&i->vertex[0].x, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[0].y, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[0].z, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[1].x, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[1].y, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[1].z, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[2].x, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[2].y, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[2].z, cp, sizeof(float)); cp += sizeof(float);

		// Skip attribute.
		cp += sizeof(short unsigned int);
    }

	in.close();

    return true;
} 


void add_box(vector<triangle>& triangles, float max_extent)
{
//	return;
	//max_extent *= 1.5f;
	
	//max_extent = 1000.0f;
//

	vertex_3_with_index v0;
	vertex_3_with_index v1;
	vertex_3_with_index v2;
	vertex_3_with_index v3;
	triangle t;

	v0.x = -max_extent; v0.y = -1; v0.z = max_extent;
	v1.x = -max_extent; v1.y = -1; v1.z = -max_extent;
	v2.x = max_extent; v2.y = -1; v2.z = -max_extent;
	v3.x = max_extent; v3.y = -1; v3.z = max_extent;

	t.vertex[0] = v0;
	t.vertex[1] = v1;
	t.vertex[2] = v2;
	triangles.push_back(t);

	t.vertex[0] = v0;
	t.vertex[1] = v2;
	t.vertex[2] = v3;
	triangles.push_back(t);

	v0.x = -max_extent; v0.y = -0.9f; v0.z = max_extent;
	v1.x = -max_extent; v1.y = -0.9f; v1.z = -max_extent;
	v2.x = max_extent; v2.y = -0.9f; v2.z = -max_extent;
	v3.x = max_extent; v3.y = -0.9f; v3.z = max_extent;

	t.vertex[0] = v2;
	t.vertex[1] = v1;
	t.vertex[2] = v0;
	triangles.push_back(t);

	t.vertex[0] = v3;
	t.vertex[1] = v2;
	t.vertex[2] = v0;
	triangles.push_back(t);



	v0.x = max_extent; v0.y = -1; v0.z = max_extent;
	v1.x = max_extent; v1.y = -1; v1.z = -max_extent;
	v2.x = max_extent; v2.y = -0.9f; v2.z = -max_extent;
	v3.x = max_extent; v3.y = -0.9f; v3.z = max_extent;

	t.vertex[0] = v0;
	t.vertex[1] = v1;
	t.vertex[2] = v2;
	triangles.push_back(t);

	t.vertex[0] = v0;
	t.vertex[1] = v2;
	t.vertex[2] = v3;
	triangles.push_back(t);

	v0.x = -max_extent; v0.y = -1; v0.z = max_extent;
	v1.x = -max_extent; v1.y = -1; v1.z = -max_extent;
	v2.x = -max_extent; v2.y = -0.9f; v2.z = -max_extent;
	v3.x = -max_extent; v3.y = -0.9f; v3.z = max_extent;

	t.vertex[0] = v2;
	t.vertex[1] = v1;
	t.vertex[2] = v0;
	triangles.push_back(t);

	t.vertex[0] = v3;
	t.vertex[1] = v2;
	t.vertex[2] = v0;
	triangles.push_back(t);



	v0.x = max_extent; v0.y = -1; v0.z = max_extent;
	v1.x = -max_extent; v1.y = -1; v1.z = max_extent;
	v2.x = -max_extent; v2.y = -0.9f; v2.z = max_extent;
	v3.x = max_extent; v3.y = -0.9f; v3.z = max_extent;

	t.vertex[0] = v2;
	t.vertex[1] = v1;
	t.vertex[2] = v0;
	triangles.push_back(t);

	t.vertex[0] = v3;
	t.vertex[1] = v2;
	t.vertex[2] = v0;
	triangles.push_back(t);

	v0.x = max_extent; v0.y = -1; v0.z = -max_extent;
	v1.x = -max_extent; v1.y = -1; v1.z = -max_extent;
	v2.x = -max_extent; v2.y = -0.9f; v2.z = -max_extent;
	v3.x = max_extent; v3.y = -0.9f; v3.z = -max_extent;

	t.vertex[0] = v0;
	t.vertex[1] = v1;
	t.vertex[2] = v2;
	triangles.push_back(t);

	t.vertex[0] = v0;
	t.vertex[1] = v2;
	t.vertex[2] = v3;
	triangles.push_back(t);

}

void scale_mesh(vector<triangle> &triangles, float max_extent)
{
	float curr_x_min = numeric_limits<float>::max();
	float curr_y_min = numeric_limits<float>::max();
	float curr_z_min = numeric_limits<float>::max();
	float curr_x_max = numeric_limits<float>::min();
	float curr_y_max = numeric_limits<float>::min();
	float curr_z_max = numeric_limits<float>::min();
	
	for(size_t i = 0; i < triangles.size(); i++)
	{
		for(size_t j = 0; j < 3; j++)
		{
			if(triangles[i].vertex[j].x < curr_x_min)
				curr_x_min = triangles[i].vertex[j].x;
				
			if(triangles[i].vertex[j].x > curr_x_max)
				curr_x_max = triangles[i].vertex[j].x;

			if(triangles[i].vertex[j].y < curr_y_min)
				curr_y_min = triangles[i].vertex[j].y;
				
			if(triangles[i].vertex[j].y > curr_y_max)
				curr_y_max = triangles[i].vertex[j].y;

			if(triangles[i].vertex[j].z < curr_z_min)
				curr_z_min = triangles[i].vertex[j].z;
				
			if(triangles[i].vertex[j].z > curr_z_max)
				curr_z_max = triangles[i].vertex[j].z;
		}			
	}
	
	float curr_x_extent = fabsf(curr_x_min - curr_x_max);
	float curr_y_extent = fabsf(curr_y_min - curr_y_max);
	float curr_z_extent = fabsf(curr_z_min - curr_z_max);

	float curr_max_extent = curr_x_extent;
	
	if(curr_y_extent > curr_max_extent)
		curr_max_extent = curr_y_extent;
		
	if(curr_z_extent > curr_max_extent)
		curr_max_extent = curr_z_extent;
	
	float scale_value = max_extent / curr_max_extent;
	
	cout << "Original max extent: " << curr_max_extent << endl;
	cout << "Scaling all vertices by a factor of: " << scale_value << endl;
	cout << "New max extent: " << max_extent << endl;

	for(size_t i = 0; i < triangles.size(); i++)
	{
		for(size_t j = 0; j < 3; j++)
		{
			triangles[i].vertex[j].x *= scale_value;
			triangles[i].vertex[j].y *= scale_value;
			triangles[i].vertex[j].z *= scale_value;
		}			
	}
}
