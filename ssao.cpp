#include "ssao.h"



int main(int argc, char **argv)
{
	srand(static_cast<unsigned int>(time(0)));

	log_system.set_max_size(20);
	log_system.add_string_to_contents("Welcome to Julia 4D 3");
	log_system.add_string_to_contents("Press the Generate mesh button to begin!");
	log_system.add_string_to_contents(" ");
	log_system.add_string_to_contents("LMB + drag -- rotate camera view");
	log_system.add_string_to_contents("RMB + drag -- adjust camera distance");


    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowSize(glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
	glutInitWindowPosition(0, 0);

    win_id = glutCreateWindow("Julia 4D 3");
 
	if(false == init())
		return 3;
	
	if (is_amd_gpu)
	{
		log_system.add_string_to_contents(" ");
		log_system.add_string_to_contents("Warning: Multithreaded compute shader support");
		log_system.add_string_to_contents("is intermittent on AMD Vega 3!");
	}

	setup_gui();

    glui->set_main_gfx_window(win_id);
	
	glutDisplayFunc(display_func);

	GLUI_Master.set_glutDisplayFunc(display_func);
	GLUI_Master.set_glutKeyboardFunc(keyboard_func);
	GLUI_Master.set_glutMouseFunc(mouse_func);
	GLUI_Master.set_glutReshapeFunc(myGlutReshape);
	GLUI_Master.set_glutIdleFunc(myGlutIdle);

	glutMotionFunc(motion_func);
	glutPassiveMotionFunc(passive_motion_func);

	glutMainLoop();
    
    return 0;
}
