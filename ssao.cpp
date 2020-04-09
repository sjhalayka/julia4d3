#include "ssao.h"


int main(int argc, char **argv)
{
	vector<triangle> triangles;

	if(argc != 2)
	{
		cout << "Example usage: " << argv[0] << " filename.stl" << endl;
		return 1;
	}	

	if(false == read_triangles_from_binary_stereo_lithography_file(triangles, argv[1]))
	{
		cout << "Error: Could not properly read file " << argv[1] << endl;
		return 2;
	}
	
	scale_mesh(triangles, 2.0f);

	//add_box(triangles, 1.0f);

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

    glutMainLoop();

    
    return 0;
}
