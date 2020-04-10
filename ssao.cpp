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
    glutInitWindowSize(glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
    win_id = glutCreateWindow(argv[0]);
	glewInit();
	
	if(false == init())
		return 3;

    glutDisplayFunc(display_func);
	//glutReshapeFunc(reshape_func);
	//glutIdleFunc(idle_func);
	glutKeyboardFunc(keyboard_func);
	glutMouseFunc(mouse_func);
	glutMotionFunc(motion_func);
	glutPassiveMotionFunc(passive_motion_func);

	GLUI_Master.set_glutReshapeFunc(myGlutReshape);

    /*** Create the side subwindow ***/

	GLUI_Master.set_glutReshapeFunc(myGlutReshape);
//	GLUI_Master.set_glutKeyboardFunc(myGlutKeyboard);

	/*** Create the side subwindow ***/
	glui = GLUI_Master.create_glui_subwindow(win_id, GLUI_SUBWINDOW_RIGHT);

	glui->add_button(const_cast<char*>("Generate"), 0, generate_cancel_button_func);
	glui->add_button(const_cast<char*>("Export to STL"), 0, export_button_func);

	glui->add_separator();

	equation_edittext = glui->add_edittext(const_cast<char*>("Equation:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("Z = Z*Z + C"), 3, control_cb);

	//	equation_edittext->set_w(150);

	glui->add_separator();

	obj_panel = glui->add_panel(const_cast<char*>("C"));
	c_x_edittext = glui->add_edittext_to_panel(obj_panel, const_cast<char*>("C.x:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("0.2866"), 3, control_cb);
	c_y_edittext = glui->add_edittext_to_panel(obj_panel, const_cast<char*>("C.y:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("0.5133"), 3, control_cb);
	c_z_edittext = glui->add_edittext_to_panel(obj_panel, const_cast<char*>("C.z:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("0.46"), 3, control_cb);
	c_w_edittext = glui->add_edittext_to_panel(obj_panel, const_cast<char*>("C.w:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("0.2467"), 3, control_cb);

	obj_panel2 = glui->add_panel(const_cast<char*>("Various parameters"));
	z_w_edittext = glui->add_edittext_to_panel(obj_panel2, const_cast<char*>("Z.w:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("0.0"), 3, control_cb);
	iterations_edittext = glui->add_edittext_to_panel(obj_panel2, const_cast<char*>("Iterations:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("8"), 3, control_cb);
	resolution_edittext = glui->add_edittext_to_panel(obj_panel2, const_cast<char*>("Resolution:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("100"), 3, control_cb);
	infinity_edittext = glui->add_edittext_to_panel(obj_panel2, const_cast<char*>("Infinity:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("4.0"), 3, control_cb);

	obj_panel3 = glui->add_panel(const_cast<char*>("Space min/max"));
	x_min_edittext = glui->add_edittext_to_panel(obj_panel3, const_cast<char*>("X min:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("-1.5"), 3, control_cb);
	y_min_edittext = glui->add_edittext_to_panel(obj_panel3, const_cast<char*>("Y min:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("-1.5"), 3, control_cb);
	z_min_edittext = glui->add_edittext_to_panel(obj_panel3, const_cast<char*>("Z min:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("-1.5"), 3, control_cb);

	x_max_edittext = glui->add_edittext_to_panel(obj_panel3, const_cast<char*>("X max:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("1.5"), 3, control_cb);
	y_max_edittext = glui->add_edittext_to_panel(obj_panel3, const_cast<char*>("Y max:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("1.5"), 3, control_cb);
	z_max_edittext = glui->add_edittext_to_panel(obj_panel3, const_cast<char*>("Z max:"), GLUI_EDITTEXT_TEXT, const_cast<char*>("1.5"), 3, control_cb);



    /**** Link windows to GLUI, and register idle callback ******/

    glui->set_main_gfx_window(win_id);



    /**** We register the idle callback with GLUI, *not* with GLUT ****/
    GLUI_Master.set_glutIdleFunc(myGlutIdle);




    glutMainLoop();

    
    return 0;
}
