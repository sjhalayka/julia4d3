#include "ssao.h"



int main(int argc, char **argv)
{
	vector<triangle> input_triangles;

	if(false == read_triangles_from_binary_stereo_lithography_file(input_triangles, "out100.stl"))
	{
		cout << "Error: Could not properly read file " << endl;
		return 2;
	}

	scale_mesh(input_triangles, 2.0f);


	vector<triangle> pedestal_triangles;

	if (false == read_triangles_from_binary_stereo_lithography_file(pedestal_triangles, "pedestal.stl"))
	{
		cout << "Error: Could not properly read file " << endl;
		return 2;
	}

	//for (size_t i = 0; i < pedestal_triangles.size(); i++)
	//{
	//	vertex_3_with_index vt;

	//	vt = pedestal_triangles[i].vertex[0];
	//	pedestal_triangles[i].vertex[0] = pedestal_triangles[i].vertex[2];
	//	pedestal_triangles[i].vertex[2] = vt;
	//}

	for (size_t i = 0; i < pedestal_triangles.size(); i++)
	{
		pedestal_triangles[i].vertex[0].y += 0.25f;
		pedestal_triangles[i].vertex[1].y += 0.25f;
		pedestal_triangles[i].vertex[2].y += 0.25f;
	}

	vector<triangle> triangles;

	for (size_t i = 0; i < input_triangles.size(); i++)
		triangles.push_back(input_triangles[i]);

	for (size_t i = 0; i < pedestal_triangles.size(); i++)
		triangles.push_back(pedestal_triangles[i]);

	get_triangle_indices_and_vertices_with_face_normals_from_triangles(triangles, triangle_indices, vertices_with_face_normals);

    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
    glutInitWindowSize(win_x, win_y);
    win_id = glutCreateWindow(argv[0]);
	glewInit();
	
	if(false == init())
		return 3;

    glutDisplayFunc(display_func);
	glutReshapeFunc(reshape_func);
	glutIdleFunc(idle_func);
	glutKeyboardFunc(keyboard_func);
	glutMouseFunc(mouse_func);
	glutMotionFunc(motion_func);
	glutPassiveMotionFunc(passive_motion_func);



	GLUI* glui = GLUI_Master.create_glui(const_cast<char*>("GLUI"), 0, 400, 50); /* name, flags,
							   x, and y */
	glui->add_statictext(const_cast<char*>("GLUI Example 2"));
	glui->add_separator();
	checkbox = glui->add_checkbox(const_cast<char*>("Wireframe"), &wireframe, 1, control_cb);
	spinner = glui->add_spinner(const_cast<char*>("Segments:"), GLUI_SPINNER_INT, &segments,
		2, control_cb);
	spinner->set_int_limits(3, 60);
	edittext = glui->add_edittext(const_cast<char*>("Text:"), GLUI_EDITTEXT_TEXT, text,
		3, control_cb);
	GLUI_Panel* obj_panel = glui->add_panel(const_cast<char*>("Object Type"));
	radio = glui->add_radiogroup_to_panel(obj_panel, &obj, 4, control_cb);
	glui->add_radiobutton_to_group(radio, const_cast<char*>("Sphere"));
	glui->add_radiobutton_to_group(radio, const_cast<char*>("Torus"));
	glui->add_button(const_cast<char*>("Quit"), 0, (GLUI_Update_CB)exit);

	glui->set_main_gfx_window(win_id);

	/* We register the idle callback with GLUI, *not* with GLUT */
	GLUI_Master.set_glutIdleFunc(myGlutIdle);




    glutMainLoop();

    
    return 0;
}
