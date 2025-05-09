#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Definitions.h"

Window wd;
Status st;
My_Polygon pg;

// GLUT callbacks
void timer(int value) {
	rotate_points_around_center_of_grivity(&pg);
	glutPostRedisplay();
	if (st.rotation_mode)
		glutTimerFunc(ROTATION_STEP, timer, 0);
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	if (pg.n_points > 0)
		draw_lines_by_points(&pg, &st);
	if (st.polygon_mode) {				// set the center of gravity to show if polygon is created
		glColor3f(CENTER_POINT_COLOR);
		glBegin(GL_POINTS);
		glVertex2f(pg.center_x, pg.center_y);
		glEnd();
	}
	glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'c':
		if (!st.rotation_mode) {
			pg.n_points = 0, st.polygon_mode = 0;
		}
		glutPostRedisplay();
		break;
	case 'p':
		if (!st.polygon_mode) {
			if (pg.n_points >= 3) {		// polygon with more then 3 points finished
				//close_line_segments(&pg);	// connect the last point with the first point
				st.polygon_mode = 1;
				update_center_of_gravity(&pg);	// calculate the center of gravity
				fprintf(stderr, "*** Polygon selection is finished!\n");
				glutPostRedisplay();
			}
			else
				fprintf(stderr, "*** Choose at least three points!\n");
		}
		break;
	case 'r':
		if (st.polygon_mode) {
			if (!st.rotation_mode) {
				update_center_of_gravity(&pg);
			}
			st.rotation_mode = 1 - st.rotation_mode;
			if (st.rotation_mode)
				glutTimerFunc(ROTATION_STEP, timer, 0);
		}
		break;
	case 'f':
		glutLeaveMainLoop(); 
		break;
	}
}

void special(int key, int x, int y) {
	if (st.rotation_mode || !st.polygon_mode) return;
	switch (key) {
	case GLUT_KEY_LEFT:
		move_points(&pg, -TRANSLATION_OFFSET, 0.0f);
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		move_points(&pg, TRANSLATION_OFFSET, 0.0f);
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		move_points(&pg, 0.0f, -TRANSLATION_OFFSET);
		glutPostRedisplay();
		break;
	case GLUT_KEY_UP:
		move_points(&pg, 0.0f, TRANSLATION_OFFSET);
		glutPostRedisplay();
		break;
	}
}

static int prev_x, prev_y;
void mousepress(int button, int state, int x, int y) {
	//if (st.rotation_mode) return;
	if (!st.polygon_mode) {
		if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
			int key_state = glutGetModifiers();
			if (key_state & GLUT_ACTIVE_CTRL) {		// if right-click and ctrl key, add points
				add_point(&pg, &wd, x, y);
				glutPostRedisplay();
			}
		}
	}
	else {
		if (button == GLUT_LEFT_BUTTON) {
			if (state == GLUT_DOWN) {
				
				prev_x = x, prev_y = y;			

				/* Calculate the distance of the cursor to center in OpenGL coordinates */
				float normx, normy, dx, dy;
				float threshold = 0.0001f;
				normx = 2.0f * ((float)x) / wd.width - 1.0f;
				normy = 2.0f * ((float)wd.height - y) / wd.height - 1.0f;
				dx = pg.center_x - normx, dy = pg.center_y - normy;
				float distance = dx * dx + dy * dy;

				if (distance <= threshold) {
					//fprintf(stderr, "### The center of gravity is clicked!\n");
					st.leftbuttonpressed = 1;
					glutPostRedisplay();	// draw with different color
				}
			}
			else if (state == GLUT_UP) {
				st.leftbuttonpressed = 0;
			}
		}
		else if (button == MOUSE_SCROLL_UP) {
			if (state == GLUT_DOWN) {
				//fprintf(stderr, "### Mouse wheel scrolled up!\n");
				scale_points(&pg, 1.1);
				glutPostRedisplay();
			}
		}
		else if (button == MOUSE_SCROLL_DOWN) {
			if (state == GLUT_DOWN) {
				//fprintf(stderr, "### Mouse wheel scrolled down!\n");
				scale_points(&pg, 0.9);
				glutPostRedisplay();
			}
		}
	}
	
}

void mousemove(int x, int y) {
	if (st.leftbuttonpressed && st.polygon_mode) {
			// if the center of gravity is clicked, move the polygon
			float delx, dely;
			delx = 2.0f * ((float)x - prev_x) / wd.width;
			dely = 2.0f * ((float)prev_y - y) / wd.height;
			prev_x = x, prev_y = y;
			move_points(&pg, delx, dely);
			update_center_of_gravity(&pg);	// update the center of gravity
			glutPostRedisplay();
	}
}
	
void reshape(int width, int height) {
	fprintf(stdout, "### The new window size is %dx%d.\n", width, height);
	wd.width = width, wd.height = height;
	glViewport(0, 0, wd.width, wd.height);
}

void close(void) {
	fprintf(stdout, "\n^^^ The control is at the close callback function now.\n\n");
}
// End of GLUT callbacks

void initialize_polygon_editor(void) {
	wd.width = 800, wd.height = 600, wd.initial_anchor_x = 500, wd.initial_anchor_y = 200;
	st.rightbuttonpressed = 0, st.rotation_mode = 0, st.polygon_mode = 0;
	pg.n_points = 0; pg.center_x = 0.0f; pg.center_y = 0.0f;
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mousepress);
	glutMotionFunc(mousemove);
	glutReshapeFunc(reshape);
	glutCloseFunc(close);
}

void initialize_renderer(void) {
	register_callbacks();

	glPointSize(5.0);
	glClearColor(BACKGROUND_COLOR, 1.0f);
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = TRUE;
	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 4
int main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 SimplefreeGLUTcode_Polygon_Editor";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'p', 'c', 'r', 'f'",
		"    - Special keys used: LEFT, RIGHT, UP, DOWN",
		"    - Mouse used: L-click, R-click and move",
		"    - Other operations: window reshape"
	};

	glutInit(&argc, argv);
	initialize_polygon_editor();

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE); // <-- Be sure to use this profile for this example code!
 //	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_RGBA);

	glutInitWindowSize(wd.width, wd.height);
	glutInitWindowPosition(wd.initial_anchor_x, wd.initial_anchor_y);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

   // glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_EXIT); // default
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	
	glutMainLoop();
	fprintf(stdout, "^^^ The control is at the end of main function now.\n\n");
	return 0;
}
