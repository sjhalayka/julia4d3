#include "ssao.h"



int main(int argc, char **argv)
{
	log_system.set_max_size(20);
	log_system.add_string_to_contents("Welcome to Julia 4D 2");

    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowSize(glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
	glutInitWindowPosition(0, 0);

    win_id = glutCreateWindow("Julia 4D 3");
	glewInit();
	
	//if (false == read_triangles_from_binary_stereo_lithography_file(triangles, "sample.stl"))
	//{
	//	cout << "Error: Could not properly read sample file " << endl;
	//	return 2;
	//}

	//stop = false;
	//get_vertices_with_face_normals_from_triangles();
	//uploaded_to_gpu = false;
 
	if(false == init())
		return 3;

    glutDisplayFunc(display_func);

	glutKeyboardFunc(keyboard_func);
	glutMouseFunc(mouse_func);
	glutMotionFunc(motion_func);
	glutPassiveMotionFunc(passive_motion_func);

	GLUI_Master.set_glutReshapeFunc(myGlutReshape);
	GLUI_Master.set_glutIdleFunc(myGlutIdle);
	
	setup_gui();

    /**** Link windows to GLUI, and register idle callback ******/

    glui->set_main_gfx_window(win_id);



    /**** We register the idle callback with GLUI, *not* with GLUT ****/





    glutMainLoop();

    
    return 0;
}
