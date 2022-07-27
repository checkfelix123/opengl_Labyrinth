/*
 * Felix Graf s2010307062
 * Hours: 9
 *
 * Description:
 *	*movement:
 *		"w": forwards
 *		"s": backwards
 *		"a": left
 *		"d": right
 *		"f": pickup
 *
 *	implementation:
 *		I used namespaces to structure the code more. Furthermore, i defined my global variables as constexpr and only used #define for macros
 *		which avoid Magicnumbers. I used "magicnumbers" in places where the creation of variables where not effective enough  
 *		random numbers are used to select between differnt labyrinths at the beginning of the game
 *
 *		the labyrinths array has two dimensions the first one is the maze, the second the block. 
 *
 *		additional information:
 *
 *		The lifting of the "ball" can be done in the main room 
 */

#include <cstdlib>

#include "GL\glew.h"
#include "GL\freeglut.h"
#include <iostream>
#include <random>


//Const variables
constexpr int ROWS_LABYRINTH	= 12;
constexpr int COLUMNS_LABYRINTH = 12;
constexpr int SIZE_BLOCK		= 4;
constexpr int GAMESPEED		    = 1;


/**
 * \brief define is used to reduce magic numbers (in array declaration
 */
#define WITH_LABYRINTH (ROWS_LABYRINTH * SIZE_BLOCK)
#define HEIGHT_LABYRINTH (COLUMNS_LABYRINTH * SIZE_BLOCK)
#define SIZE_LABYRINTH (ROWS_LABYRINTH * COLUMNS_LABYRINTH)


using namespace std;
int windowid;
// Navigation
GLdouble navX = -2;
GLdouble navY = 1;
GLdouble navZ = 0;


GLdouble mouseX = 1;
GLdouble mouseY = 0;
GLdouble mouseZ = 3;
GLdouble angle = 0;
int labyrinth_rand;

//variables used for "statemanagement"
bool is_jumping{false };
bool lifting{false};
bool lifted{false};


namespace fgraf_collections
{
	
}


int idx(int const r, int const c)
{
	// transform coordinates from 2D to 1D
	return r * COLUMNS_LABYRINTH + c;
}


//two possible labyrinths 
int labyrinths[2][SIZE_LABYRINTH] = {
	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
		1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1,
		1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1,
		1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1,
		1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1,
		1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	},

	{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1,
		1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1,
		1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1,
		1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1,
		1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
		1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1,
		1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	}

};
namespace fgraf_utils
{


	bool is_in_range(int x, int z)
	{
		return navX > (x - 1) && navX < (x + 1) && navZ >(z - 1) && navZ < (z + 1);
	}


	//used to create random numbers
	template <typename E = std::mt19937_64>
	int get_random_uniform(int const l, int const u)
	{
		static E generator{ std::random_device{}() }; //generate (atomzerfall)
		return std::uniform_int_distribution<>{l, u}(generator);
	}

	//get random number
	int get_random_number(const int num_min = 0, const int num_max = 0)
	{
		return get_random_uniform(num_min, num_max);
	}

}


namespace fgraf_objects
{

	/**
	 * TASK 3.1
	 * This method is used to create a object which then can be lifted by the player
	 */
	void object_for_lift(int x, int z) {
		glColor3d(0.6, 0.6, 0.6);
		glPushMatrix();

		if (!lifted) glTranslated(x, .2, z);
		if (lifted) glTranslated(navX, .5, navZ - 0.75);
		if (lifting && fgraf_utils::is_in_range(x, z) && !lifted) {
			lifted = true;
		}
		else if (lifting)  // If the object was grabbed and the player tries to grab it again, it will be placed back where it was.
		{
			lifted = false;
		}
		lifting = false;
		glutSolidSphere(.25, 16, 16);
		glPopMatrix();
	}
	void floor()
	{
		// draw ground
		glPushMatrix();
		glTranslated(0, 0, 0);
		glColor3f(0.3f, 0.0f, 0.0f); //Brown
		glBegin(GL_QUADS);
		glVertex3d(-400, 0, -400);
		glVertex3d(400, 0, -400);
		glVertex3d(400, 0, 400);
		glVertex3d(-400, 0, 400);
		glEnd();
		glPopMatrix();
	}

	//TASK 1
	void table(int x, int y)
	{
		// draw table with base, stand & plate

		glPushMatrix();
		glColor3f(0.1f, 0.0f, 0.0f); //Brown
		glTranslatef(x, 0.5 * SIZE_BLOCK / 2, y);
		//top
		glPushMatrix();
		glTranslatef(0, 0.0, 0);
		glRotatef(90, 1.0, 0.0, 0.0);
		glutSolidCylinder(0.5 * SIZE_BLOCK / 2, 0.025 * SIZE_BLOCK / 2, 64, 64);
		glPopMatrix();
		//stand
		glPushMatrix();
		glTranslatef(0, 0, 0);
		glRotatef(90, 1.0, 0.0, 0.0);
		glutSolidCylinder(0.025 * SIZE_BLOCK / 2, 0.5 * SIZE_BLOCK / 2, 64, 64);
		glPopMatrix();
		glPopMatrix();
	}

	//TASK 1
	void chair(int x, int y)
	{
		// draw table with base, stand & plate

		glPushMatrix();
		glColor3f(0.1f, 0.0f, 0.0f); //Brown
		glTranslatef(x, 0.25 * SIZE_BLOCK / 2, y);
		//top
		glPushMatrix();
		glTranslatef(0, 0.0, 0);
		glRotatef(90, 1.0, 0.0, 0.0);
		glutSolidCylinder(0.25 * SIZE_BLOCK / 2, 0.025 * SIZE_BLOCK / 2, 64, 64);
		glPopMatrix();
		//stand
		glPushMatrix();
		glTranslatef(-0.125 * SIZE_BLOCK / 2, 0.0, -0.125 * SIZE_BLOCK / 2);
		glRotatef(90, 1.0, 0.0, 0.0);
		glutSolidCylinder(0.025 * SIZE_BLOCK / 2, 0.25 * SIZE_BLOCK / 2, 64, 64);

		glPopMatrix();
		//stand
		glPushMatrix();
		glTranslatef(-0.125 * SIZE_BLOCK / 2, 0.0, 0.125 * SIZE_BLOCK / 2);
		glRotatef(90, 1.0, 0.0, 0.0);
		glutSolidCylinder(0.025 * SIZE_BLOCK / 2, 0.25 * SIZE_BLOCK / 2, 64, 64);
		glPopMatrix();
		//stand
		glPushMatrix();
		glTranslatef(0.125 * SIZE_BLOCK / 2, 0.0, -0.125 * SIZE_BLOCK / 2);
		glRotatef(90, 1.0, 0.0, 0.0);
		glutSolidCylinder(0.025 * SIZE_BLOCK / 2, 0.25 * SIZE_BLOCK / 2, 64, 64);
		glPopMatrix();
		//stand
		glPushMatrix();
		glTranslatef(0.125 * SIZE_BLOCK / 2, 0.0, 0.125 * SIZE_BLOCK / 2);
		glRotatef(90, 1.0, 0.0, 0.0);
		glutSolidCylinder(0.025 * SIZE_BLOCK / 2, 0.25 * SIZE_BLOCK / 2, 64, 64);
		glPopMatrix();
		glPopMatrix();
	}

	void objects()
	{
		table(6 * SIZE_BLOCK, 4 * SIZE_BLOCK);
		chair(7 * SIZE_BLOCK, 4 * SIZE_BLOCK);
		chair(6 * SIZE_BLOCK, 5 * SIZE_BLOCK);
	}


}



namespace fgraf_game
{
	/**
	 * this method converts array indexes to real world coordinates.
	 */
	void get_coordinates(size_t i, size_t j, GLdouble& x, GLdouble& z)
	{
		x = i * static_cast<GLdouble>(SIZE_BLOCK);


		z = j * static_cast<GLdouble>(SIZE_BLOCK);
	}




	/*
	 * This function is used to create a new
	 * Block of a labyrinth
	 */
	void create_block(size_t i, size_t j)
	{

		GLdouble x, z;
		glPushMatrix();
		get_coordinates(i, j, x, z);
		glTranslated(x, 0, z);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glColor3d(0.7, 0.7, 0.7); 
		glutSolidCube(SIZE_BLOCK);
		glPopMatrix();
	}

	/**
	 *	TASK 2
	 * Rendering of labyrinth
	 */
	void labyrinth()
	{
		//iterate through all rows and cols and place blocks on all indices where the array is 1
		for (size_t i = 0; i < ROWS_LABYRINTH; i++)
		{
			for (size_t j = 0; j < COLUMNS_LABYRINTH; j++)
			{
				if (labyrinths[labyrinth_rand][idx(i, j)] == 1)
				{
					create_block(i, j); //create new labyrinth block 
				}
			}
		}
	}

	/**
	 * This method is used to check if a move is possible or not.
	 */
	bool is_move_possible(GLdouble x, GLdouble z)
	{

		//select correct maze and then get the correct index by dividing by block size. The coordinates need to be rounded otherwise this mechanism dont work
		const int pos = labyrinths[labyrinth_rand][
			idx(
				std::round(std::round(x)/SIZE_BLOCK), 
				std::round(std::round(z) / SIZE_BLOCK)
			)
		];

		return pos ==  0;
	}
	void check_is_move_possible(GLdouble x, GLdouble z)
	{
		if (is_move_possible(x, z))
		{
			navX = x;
			navZ = z;
		}
	}
	//TASK 1.4
	void animate_jump()
	{
		static bool toggle = true;
		if (mouseY < SIZE_BLOCK - 1 && toggle) //jumping upwards
		{
			mouseY += 0.025;
		}
		else if (mouseY > 0) // else move player down
		{
			mouseY -= 0.05;
			toggle = false;
		}
		else
		{
			is_jumping = false;
			toggle = true;
		}
	}

}



/**
 * This method renders the game (scene)
 */

void renderScene(void)
{
	glMatrixMode(GL_MODELVIEW);

	//glClear — clear buffers to preset values
	glClear(GL_COLOR_BUFFER_BIT);//color values are set back, "previous rendered scenes get removed"
	glClear(GL_DEPTH_BUFFER_BIT);//depth values  are set back to one.

	glLoadIdentity();
	// set camera position
	gluLookAt(navX, navY + mouseY, navZ, static_cast<GLdouble>(navX) + mouseX * SIZE_BLOCK, mouseY / 2, navZ - (mouseZ * SIZE_BLOCK), 0.0f, 1.0f, 0.0f);
	if (is_jumping)
	{
		fgraf_game::animate_jump();
	}
	//Render the labyrinth
	fgraf_game::labyrinth();

	fgraf_objects::objects();
	fgraf_objects::floor();
	fgraf_objects::object_for_lift(7 * SIZE_BLOCK, 5 * SIZE_BLOCK);
	glutSwapBuffers();
}


/**
 * TASK 1.3
 * This method is used to enable the player to move
 */
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		fgraf_game::check_is_move_possible(navX - mouseZ * GAMESPEED, navZ - mouseX * GAMESPEED); //left
		break;
	case 'd':
		fgraf_game::check_is_move_possible(navX + mouseZ * GAMESPEED, navZ + mouseX * GAMESPEED); //right
		break;
	case 'w':
		fgraf_game::check_is_move_possible(navX + mouseX * GAMESPEED, navZ - mouseZ * GAMESPEED); //forward
		break;
	case 's':
		fgraf_game::check_is_move_possible(navX - mouseX * GAMESPEED, navZ + mouseZ * GAMESPEED); //backwords
		break;

	case 'f':
		lifting = true;
		break;
	case 32:
		is_jumping = true;
		break;
	case 27: // Escape key
		glutDestroyWindow(windowid);
		exit(0);
		break;
	}

	glutPostRedisplay();
}



/*-[Reshape Callback]--------------------------------------------------------*/
void reshapeFunc(int x, int y)
{
	if (y == 0 || x == 0) return; //Nothing is visible then return

	glMatrixMode(GL_PROJECTION); //Set a new projection matrix
	glLoadIdentity();
	gluPerspective(60.0, static_cast<GLdouble>(x) / static_cast<GLdouble>(y), 0.5, 60.0);
	glViewport(0, 0, x, y); //Use the whole window for rendering
}

/**
 *	TASK 3
 *	Realisation of mousemovement. Mouse is fixed inside the middle of screen.
 */
void mouseMovement(int x, int y)
{
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	angle += (width / 2 - static_cast<GLdouble>(x)) * 0.0005;
	mouseX = sin(angle);
	mouseZ = -cos(angle);
	glutWarpPointer(width / 2, height / 2);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	navX = 8 * SIZE_BLOCK;
	navY = 1;
	navZ = 6 * SIZE_BLOCK;
	angle = 0;
	labyrinth_rand = fgraf_utils::get_random_number(0, 1);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); // rgba color schema, double buffering
	glutInitWindowPosition(500, 500); // determines the initial position of the window
	glutInitWindowSize(800, 600); // determines the size of the window
	windowid = glutCreateWindow("OpenGL"); // create and name window
	glutFullScreen(); // set full screen mode
	glutSetCursor(GLUT_CURSOR_NONE); // show no cursor

	glutReshapeFunc(reshapeFunc);
	glutIdleFunc(renderScene);
	glutPassiveMotionFunc(mouseMovement);


	glutDisplayFunc(renderScene);
	glutKeyboardFunc(keyboard);

	glEnable(GL_DEPTH_TEST);
	glutMainLoop(); // start the main loop of GLUT

	return EXIT_SUCCESS;
}
