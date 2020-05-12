#include "ssao.h"



int main(int argc, char **argv)
{
	srand(static_cast<unsigned int>(time(0)));

	log_system.set_max_size(30);
	log_system.add_string_to_contents("Welcome to Julia 4D 3 v1.7");
	log_system.add_string_to_contents("Press the Generate mesh button to begin!");
	log_system.add_string_to_contents("-------------------------------------------------------");
	log_system.add_string_to_contents("LMB + drag: rotate camera view");
	log_system.add_string_to_contents("RMB + drag: adjust camera distance");
	log_system.add_string_to_contents("-------------------------------------------------------");

    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
	glutInitWindowPosition(0, 0);

    win_id = glutCreateWindow("Julia 4D 3");
 
	if(false == init())
		return 1;
	



	setup_gui();

    glui->set_main_gfx_window(win_id);

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
