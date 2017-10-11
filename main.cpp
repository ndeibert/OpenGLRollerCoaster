/*
 *  CSCI 441, Computer Graphics, Fall 2017
 *
 *  Project: lab03
 *  File: main.cpp
 *
 *	Author: Dr. Jeffrey Paone - Fall 2015
 *
 *  Description:
 *      Contains the base code for 3D Bezier Curve visualizer.
 *
 */

// HEADERS /////////////////////////////////////////////////////////////////////

// include the OpenGL library header
#ifdef __APPLE__				// if compiling on Mac OS
#include <OpenGL/gl.h>
#else							// if compiling on Linux or Windows OS
#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>			// include GLFW framework header

#include <CSCI441/objects.hpp> // for our 3D objects

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <math.h>				// for cos(), sin() functionality
#include <stdio.h>			// for printf functionality
#include <stdlib.h>			// for exit functionality
#include <time.h>			  // for time() functionality

#include <fstream>			// for file I/O
#include <sstream>
#include <vector>				// for vector
#include <string>     // std::string, std::stoi
using namespace std;

//*************************************************************************************
//
// Global Parameters

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
int windowWidth = 640, windowHeight = 480;

int leftMouseButton;    	 									// status of the mouse button
glm::vec2 mousePosition;				            // last known X and Y of the mouse

glm::vec3 camPos;            							 	// camera position in cartesian coordinates
glm::vec3 camAngles;               					// camera DIRECTION in spherical coordinates stored as (theta, phi, radius)
glm::vec3 camDir; 			                    // camera DIRECTION in cartesian coordinates

vector<glm::vec3> controlPoints;
vector<glm::vec3> coeffs;
float trackPointVal = 0.0f;
int numSegments = 0;

int currentCurvePointParametric = 0;
int currentCurvePointArc = 0;

vector<glm::vec3> curvePoints;
int r = 0;

//*************************************************************************************
//
// Helper Function

// inspired by https://stackoverflow.com/questions/236129/most-elegant-way-to-split-a-string
vector<string> split(const std::string &s, char delim) {
	vector<string> fields;
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        fields.push_back(item);
    }
	return fields;
}

// loadControlPoints() /////////////////////////////////////////////////////////
//
//  Load our control points from file and store them in
//	the global variable controlPoints
//
////////////////////////////////////////////////////////////////////////////////
bool loadControlPoints( char* filename ) {
	// TODO #02: read in control points from file.  Make sure the file can be
	// opened and handle it appropriately.
	ifstream file ( filename );
	int num_points;
	if (file.good()) {
		file >> num_points;
	}
	else {
		printf ("Invalid CSV file");
		exit(1);
	}
	for (int i = 0; i < num_points; i++) {
		string line;
		file >> line;
		// printf("%s\n", line.c_str());
		vector<string> fields = split(line, ',');
		
		float x,y,z;
		x = atoi(fields.at(0).c_str());
		y = atoi(fields.at(1).c_str());
		z = atoi(fields.at(2).c_str());
		controlPoints.push_back(glm::vec3(x,y,z));
		printf("%f %f %f\n", x, y, z);
	}

	return true;
}
void loadCurvePoints() {
	int n = 0;
	while(n + 3 <= controlPoints.size() - 1){
		glm::vec3 lineFrom = controlPoints.at(n);
		glm::vec3 lineTo;
		for(float t = 0; t < 1; t+=.01) {
			glm::vec3 a = ((-1.0f * controlPoints.at(n)) + (3.0f*controlPoints.at(n + 1)) - (3.0f*controlPoints.at(n + 2)) + controlPoints.at(n + 3)) * (t * t * t);
			glm::vec3 b = ((3.0f*controlPoints.at(n)) - (6.0f*controlPoints.at(n + 1)) + (3.0f*controlPoints.at(n + 2))) * (t*t);
			glm::vec3 c = ((-3.0f*controlPoints.at(n)) + (3.0f*controlPoints.at(n + 1))) * t;
			glm::vec3 d =  controlPoints.at(n);
			glm::vec3 lineTo = a + b + c + d;
			lineFrom = lineTo;
			curvePoints.push_back(lineTo);
		}
		n+=3;
	}
}

void drawCoaster(){
	glm::mat4 transMtx;
	glDisable( GL_LIGHTING );
	for(int i = 0; i < controlPoints.size(); i++) {
		glColor3f(0.000, 0.502, 0.000);
		transMtx = glm::translate(glm::mat4(), glm::vec3(controlPoints.at(i).x, controlPoints.at(i).y, controlPoints.at(i).z));
	    glMultMatrixf( &transMtx[0][0] );
		//CSCI441::drawSolidSphere( .333, 10, 20 );
		glMultMatrixf( &( glm::inverse( transMtx ) )[0][0] );
		
	}
	for(int i = 0; i < controlPoints.size() - 1; i++) {
		glColor3f(1.000, 1.000, 0.000);
		glLineWidth(5);
		glBegin(GL_LINES);
		//glVertex3f(controlPoints.at(i).x, controlPoints.at(i).y, controlPoints.at(i).z);
		//glVertex3f(controlPoints.at(i + 1).x, controlPoints.at(i + 1).y, controlPoints.at(i + 1).z);
		glEnd();
		glLineWidth(1);
		
	}
	glEnable( GL_LIGHTING );
	int n = 0;
	while(n + 3 <= controlPoints.size() - 1){
		glm::vec3 lineFrom = controlPoints.at(n);
		glm::vec3 lineTo;
		glDisable( GL_LIGHTING );
		for(float t = 0; t < 1; t+=.01) {
			glColor3f(0.000, 0.000, 1.000);
			glLineWidth(5);
			glm::vec3 a = ((-1.0f * controlPoints.at(n)) + (3.0f*controlPoints.at(n + 1)) - (3.0f*controlPoints.at(n + 2)) + controlPoints.at(n + 3)) * (t * t * t);
			glm::vec3 b = ((3.0f*controlPoints.at(n)) - (6.0f*controlPoints.at(n + 1)) + (3.0f*controlPoints.at(n + 2))) * (t*t);
			glm::vec3 c = ((-3.0f*controlPoints.at(n)) + (3.0f*controlPoints.at(n + 1))) * t;
			glm::vec3 d =  controlPoints.at(n);
			glm::vec3 lineTo = a + b + c + d;
			glBegin(GL_LINES);
			glVertex3f(lineFrom.x,lineFrom.y,lineFrom.z);
			glVertex3f(lineTo.x,lineTo.y,lineTo.z);
			glEnd();
			lineFrom = lineTo;
			glLineWidth(1);
		}
		glEnable( GL_LIGHTING );
		n+=3;
	}
	if(currentCurvePointParametric >= curvePoints.size() - 1) {
		currentCurvePointParametric = 0;
	}
	if(currentCurvePointArc >= curvePoints.size() - 1) {
		currentCurvePointArc = 0;
	}
	glm::mat4 transMtx2 = glm::translate(glm::mat4(), curvePoints.at(currentCurvePointParametric));
	glMultMatrixf( &transMtx2[0][0] );
	CSCI441::drawSolidSphere( .333, 10, 20 );
	glMultMatrixf( &( glm::inverse( transMtx2 ) )[0][0] );
	currentCurvePointParametric++;
	
	if(r % 50 == 0){
		glm::mat4 transMtx3 = glm::translate(glm::mat4(), curvePoints.at(currentCurvePointArc));
		glMultMatrixf( &transMtx3[0][0] );
		CSCI441::drawSolidSphere( .333, 10, 20 );
		glMultMatrixf( &( glm::inverse( transMtx3 ) )[0][0] );
		currentCurvePointArc+=100;
	}
	else {
		glm::mat4 transMtx3 = glm::translate(glm::mat4(), curvePoints.at(currentCurvePointArc));
		glMultMatrixf( &transMtx3[0][0] );
		CSCI441::drawSolidSphere( .333, 10, 20 );
		glMultMatrixf( &( glm::inverse( transMtx3 ) )[0][0] );
	}
	
	r++;
		
}

// recomputeOrientation() //////////////////////////////////////////////////////
//
// This function updates the camera's position in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraTheta, cameraPhi, or cameraRadius is updated.
//
////////////////////////////////////////////////////////////////////////////////
void recomputeOrientation() {
    camDir.x =  sinf(camAngles.x)*sinf(camAngles.y);
    camDir.z = -cosf(camAngles.x)*sinf(camAngles.y);
    camDir.y = -cosf(camAngles.y);

    //and normalize this directional vector!
    camDir = glm::normalize( camDir );
}

// evaluateBezierCurve() ////////////////////////////////////////////////////////
//
// Computes a location along a Bezier Curve.
//
////////////////////////////////////////////////////////////////////////////////
glm::vec3 evaluateBezierCurve( glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t ) {
	// TODO #06: Compute a point along a Bezier curve
	glm::vec3 a = -1.0f * p0 + 3.0f * p1 - 3.0f * p2 + p3;
	glm::vec3 b = 3.0f * p0 - 6.0f * p1 + 3.0f * p2;
	glm::vec3 c = -3.0f * p0 + 3.0f * p1;
	glm::vec3 d = p0;
	
	glm::vec3 point = (t * t * t) * a + (t * t) * b + t * c + d;
	
	return point;
}

// renderBezierCurve() //////////////////////////////////////////////////////////
//
// Responsible for drawing a Bezier Curve as defined by four control points.
//  Breaks the curve into n segments as specified by the resolution.
//
////////////////////////////////////////////////////////////////////////////////
void renderBezierCurve( glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int resolution ) {
    // TODO #05: Draw the Bezier Curve!
}

//*************************************************************************************
// Event Callbacks

//
//	void error_callback( int error, const char* description )
//
//		We will register this function as GLFW's error callback.
//	When an error within GLFW occurs, GLFW will tell us by calling
//	this function.  We can then print this info to the terminal to
//	alert the user.
//
static void error_callback( int error, const char* description ) {
	fprintf( stderr, "[ERROR]: %s\n", description );
}

static void keyboard_callback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
	//because the direction vector is unit length, and we probably don't want
	//to move one full unit every time a button is pressed, just create a constant
	//to keep track of how far we want to move at each step. you could make
	//this change w.r.t. the amount of time the button's held down for
	//simple scale-sensitive movement!
	float movementConstant = 0.3f;

	if( action == GLFW_PRESS || action == GLFW_REPEAT ) {
		switch( key ) {
			case GLFW_KEY_ESCAPE:
			case GLFW_KEY_Q:
				exit(EXIT_SUCCESS);

			//move forward!
			case GLFW_KEY_W:
				//that's as simple as just moving along the direction.
				camPos.x += camDir.x*movementConstant;
				camPos.y += camDir.y*movementConstant;
				camPos.z += camDir.z*movementConstant;
				break;

			//move backwards!
			case GLFW_KEY_S:
				//just move BACKWARDS along the direction.
				camPos.x -= camDir.x*movementConstant;
				camPos.y -= camDir.y*movementConstant;
				camPos.z -= camDir.z*movementConstant;
				break;
		}
	}
}

//
//	void cursor_callback
//
//	handles mouse movement.  keeps global state of our mouse cursor.
//		active motion with left mouse button pans our free cam
//
static void cursor_callback( GLFWwindow *window, double x, double y ) {
	if(leftMouseButton == GLFW_PRESS) {
		camAngles.x += (x - mousePosition.x)*0.005;
		camAngles.y += (mousePosition.y - y)*0.005;

		// make sure that phi stays within the range (0, M_PI)
		if(camAngles.y <= 0)
				camAngles.y = 0+0.001;
		if(camAngles.y >= M_PI)
				camAngles.y = M_PI-0.001;

		recomputeOrientation();     //update camera (x,y,z) based on (theta,phi,radius)
	}

	mousePosition.x = x;
	mousePosition.y = y;
}

//
//	void mouse_button_callback
//
//	handles mouse clicks.  keeps global state of our left mouse button (pressed or released)
//
static void mouse_button_callback( GLFWwindow *window, int button, int action, int mods ) {
	if( button == GLFW_MOUSE_BUTTON_LEFT ) {
		leftMouseButton = action;
	}
}




// drawGrid() //////////////////////////////////////////////////////////////////
//
//  Function to draw a grid in the XZ-Plane using OpenGL 2D Primitives (GL_LINES)
//
////////////////////////////////////////////////////////////////////////////////
void drawGrid() {
    /*
     *	We will get to why we need to do this when we talk about lighting,
     *	but for now whenever we want to draw something with an OpenGL
     *	Primitive - like a line, quad, point - we need to disable lighting
     *	and then reenable it for use with the GLUT 3D Primitives.
     */
    glDisable( GL_LIGHTING );

    // draw our grid....what? triple nested for loops!  crazy!  but it works :)
    glColor3f( 1, 1, 1 );
    for( int dir = 0; dir < 2; dir++ ) {
        for( int i = -5; i < 6; i++ ) {
            glBegin( GL_LINE_STRIP ); {
                for( int j = -5; j < 6; j++ )
                    glVertex3f( dir < 1 ? i : j,
																0,
																dir < 1 ? j : i );
            }; glEnd();
        }
    }

    /*
     *	As noted above, we are done drawing with OpenGL Primitives, so we
     *	must turn lighting back on.
     */
    glEnable( GL_LIGHTING );
}

// renderScene() ///////////////////////////////////////////////////////////////
//
//  GLUT callback for scene rendering. Sets up the modelview matrix, renders
//      a scene to the back buffer, and switches the back buffer with the
//      front buffer (what the user sees).
//
////////////////////////////////////////////////////////////////////////////////
void renderScene(void)  {
	
	drawGrid();				// first draw our grid

	glm::mat4 transMtx1 = glm::translate(glm::mat4(), glm::vec3(-4.0f, 0.0f, 0.0f));
	glMultMatrixf( &transMtx1[0][0] );
	drawCoaster();
	glMultMatrixf( &( glm::inverse( transMtx1 ) )[0][0] );
}

//*************************************************************************************
//
// Setup Functions

//
//  void setupGLFW()
//
//      Used to setup everything GLFW related.  This includes the OpenGL context
//	and our window.
//
GLFWwindow* setupGLFW() {
	// set what function to use when registering errors
	// this is the ONLY GLFW function that can be called BEFORE GLFW is initialized
	// all other GLFW calls must be performed after GLFW has been initialized
	glfwSetErrorCallback( error_callback );

	// initialize GLFW
	if( !glfwInit() ) {
		fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW initialized\n" );
	}

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );	// request OpenGL v2.X
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );	// request OpenGL v2.1
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );		// do not allow our window to be able to be resized

	// create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow( windowWidth, windowHeight, "CSCI441 Roller Coaster Tycoon", NULL, NULL );
	if( !window ) {						// if the window could not be created, NULL is returned
		fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
		glfwTerminate();
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW Window created\n" );
	}

	glfwMakeContextCurrent(window);		// make the created window the current window
	glfwSwapInterval(1);				     	// update our screen after at least 1 screen refresh

	glfwSetKeyCallback( window, keyboard_callback );							// set our keyboard callback function
	glfwSetCursorPosCallback( window, cursor_callback );					// set our cursor position callback function
	glfwSetMouseButtonCallback( window, mouse_button_callback );	// set our mouse button callback function

	return window;						       // return the window that was created
}

//
//  void setupOpenGL()
//
//      Used to setup everything OpenGL related.  For now, the only setting
//	we need is what color to make the background of our window when we clear
//	the window.  In the future we will be adding many more settings to this
//	function.
//
void setupOpenGL() {
	// tell OpenGL to perform depth testing with the Z-Buffer to perform hidden
	//		surface removal.  We will discuss this more very soon.
  glEnable( GL_DEPTH_TEST );

	//******************************************************************
  // this is some code to enable a default light for the scene;
  // feel free to play around with this, but we won't talk about
  // lighting in OpenGL for another couple of weeks yet.
  float lightCol[4] = { 1, 1, 1, 1};
  float ambientCol[4] = { 0.0, 0.0, 0.0, 1.0 };
  float lPosition[4] = { 10, 10, 10, 1 };
  glLightfv( GL_LIGHT0, GL_POSITION,lPosition );
  glLightfv( GL_LIGHT0, GL_DIFFUSE,lightCol );
  glLightfv( GL_LIGHT0, GL_AMBIENT, ambientCol );
  glEnable( GL_LIGHTING );
  glEnable( GL_LIGHT0 );

  // tell OpenGL not to use the material system; just use whatever we
	// pass with glColor*()
  glEnable( GL_COLOR_MATERIAL );
  glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
	//******************************************************************

  // tells OpenGL to blend colors across triangles. Once lighting is
  // enabled, this means that objects will appear smoother - if your object
  // is rounded or has a smooth surface, this is probably a good idea;
  // if your object has a blocky surface, you probably want to disable this.
  glShadeModel( GL_SMOOTH );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// set the clear color to black
}

//
//	setupScene()
//
//	setup everything specific to our scene.  in this case,
//	position our camera
//
void setupScene() {
	// give the camera a scenic starting point.
	camPos.x = 6;
	camPos.y = 4;
	camPos.z = 3;
	camAngles.x = -M_PI / 3.0f;	// theta
	camAngles.y = M_PI / 2.8f;	// phi
	recomputeOrientation();
}

///*************************************************************************************
//
// Our main function

//
//	int main( int argc, char *argv[] )
//
//		Really you should know what this is by now.  We will make use of the parameters later
//
int main( int argc, char *argv[] ) {
	// TODO #01: make sure a control point CSV file was passed in.  Then read the points from file
	char* csv_file_location;
	if (argc < 2) {
		printf("CSV File argument required.");
		exit(1);
	}
	else {
		csv_file_location = argv[1];
	}
	
	loadControlPoints(csv_file_location);

	// GLFW sets up our OpenGL context so must be done first
	GLFWwindow *window = setupGLFW();	// initialize all of the GLFW specific information releated to OpenGL and our window
	setupOpenGL();										// initialize all of the OpenGL specific information
	setupScene();											// initialize objects in our scene

	fprintf(stdout, "[INFO]: /--------------------------------------------------------\\\n");
	fprintf(stdout, "[INFO]: | OpenGL Information                                     |\n");
	fprintf(stdout, "[INFO]: |--------------------------------------------------------|\n");
	fprintf(stdout, "[INFO]: |   OpenGL Version:  %35s |\n", glGetString(GL_VERSION));
	fprintf(stdout, "[INFO]: |   OpenGL Renderer: %35s |\n", glGetString(GL_RENDERER));
	fprintf(stdout, "[INFO]: |   OpenGL Vendor:   %35s |\n", glGetString(GL_VENDOR));
	fprintf(stdout, "[INFO]: \\--------------------------------------------------------/\n");

	//  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
	//	until the user decides to close the window and quit the program.  Without a loop, the
	//	window will display once and then the program exits.
	loadCurvePoints();
	while( !glfwWindowShouldClose(window) ) {	// check if the window was instructed to be closed
    glDrawBuffer( GL_BACK );				// work with our back frame buffer
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the window

		// update the projection matrix based on the window size
		// the GL_PROJECTION matrix governs properties of the view coordinates;
		// i.e. what gets seen - use a perspective projection that ranges
		// with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
		glm::mat4 projMtx = glm::perspective( 45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.001f, 1000.0f );
		glMatrixMode( GL_PROJECTION );	// change to the Projection matrix
		glLoadIdentity();				// set the matrix to be the identity
		glMultMatrixf( &projMtx[0][0] );// load our orthographic projection matrix into OpenGL's projection matrix state

		// Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
		// when using a Retina display the actual window can be larger than the requested window.  Therefore
		// query what the actual size of the window we are rendering to is.
		GLint framebufferWidth, framebufferHeight;
		glfwGetFramebufferSize( window, &framebufferWidth, &framebufferHeight );

		// update the viewport - tell OpenGL we want to render to the whole window
		glViewport( 0, 0, framebufferWidth, framebufferHeight );

		glMatrixMode( GL_MODELVIEW );	// make the ModelView matrix current to be modified by any transformations
		glLoadIdentity();							// set the matrix to be the identity

		// set up our look at matrix to position our camera
		glm::mat4 viewMtx = glm::lookAt( camPos,
								 										 camPos + camDir,
							 	 									 	 glm::vec3(  0,  1,  0 ) );
		// multiply by the look at matrix - this is the same as our view martix
		glMultMatrixf( &viewMtx[0][0] );

		renderScene();					// draw everything to the window

		glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();				// check for any events and signal to redraw screen

		trackPointVal += 0.01f;
		if( trackPointVal > numSegments )
			trackPointVal = 0.0f;
	}

	glfwDestroyWindow( window );// clean up and close our window
	glfwTerminate();						// shut down GLFW to clean up our context

	return EXIT_SUCCESS;				// exit our program successfully!
}
