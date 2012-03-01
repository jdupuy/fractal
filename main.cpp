////////////////////////////////////////////////////////////////////////////////
// \author   Jonathan Dupuy
//
////////////////////////////////////////////////////////////////////////////////

// enable gui
#define _ANT_ENABLE

// GL libraries
#include "glew.hpp"
#include "GL/freeglut.h"

#ifdef _ANT_ENABLE
#	include "AntTweakBar.h"
#endif // _ANT_ENABLE

// Custom libraries
#include "Algebra.hpp"      // Basic algebra library
#include "Transform.hpp"    // Basic transformations
#include "Framework.hpp"    // utility classes/functions

// Standard librabries
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <cmath>


////////////////////////////////////////////////////////////////////////////////
// Global variables
//
////////////////////////////////////////////////////////////////////////////////

// Constants
const float PI   = 3.14159265;
const float FOVY = PI*0.25f;

enum {
	// buffers
	BUFFER_QUAD_VERTICES = 0,
	BUFFER_COUNT,

	// vertex arrays
	VERTEX_ARRAY_QUAD = 0,
	VERTEX_ARRAY_COUNT,

	// programs
	PROGRAM_MANDELBROT = 0,
	PROGRAM_TRICORN,
	PROGRAM_BURNING_SHIP,
	PROGRAM_COUNT
};

// OpenGL objects
GLuint *buffers      = NULL;
GLuint *vertexArrays = NULL;
GLuint *programs     = NULL;

bool mouseLeft  = false;
bool mouseRight = false;

GLfloat deltaTicks = 0.0f;
GLfloat speed = 0.0f;
GLfloat scaleExp = 0.3f;
GLint exponent = 2;      // fractal exponent
Vector2 offset = Vector2(0,0);
GLint fractalProgram = PROGRAM_MANDELBROT;

#ifdef _ANT_ENABLE
GLint iterationCnt = 33;
#endif

////////////////////////////////////////////////////////////////////////////////
// Functions
//
////////////////////////////////////////////////////////////////////////////////

void set_exponent() {
	static GLint mandelLoc = glGetUniformLocation(programs[PROGRAM_MANDELBROT],
	                                              "uExponent");
	static GLint triLoc    = glGetUniformLocation(programs[PROGRAM_TRICORN],
	                                              "uExponent");
	static GLint shipLoc   = glGetUniformLocation(programs[PROGRAM_BURNING_SHIP],
	                                              "uExponent");

	glUseProgram(programs[PROGRAM_MANDELBROT]);
		glUniform1f(mandelLoc, exponent);
	glUseProgram(programs[PROGRAM_TRICORN]);
		glUniform1f(triLoc, exponent);
	glUseProgram(programs[PROGRAM_BURNING_SHIP]);
		glUniform1f(shipLoc, exponent);
	glUseProgram(0);

}

void set_iteration_count() {
	static GLint mandelLoc = glGetUniformLocation(programs[PROGRAM_MANDELBROT],
	                                              "uIterationCnt");
	static GLint triLoc    = glGetUniformLocation(programs[PROGRAM_TRICORN],
	                                              "uIterationCnt");
	static GLint shipLoc   = glGetUniformLocation(programs[PROGRAM_BURNING_SHIP],
	                                              "uIterationCnt");

	glUseProgram(programs[PROGRAM_MANDELBROT]);
		glUniform1f(mandelLoc, iterationCnt);
	glUseProgram(programs[PROGRAM_TRICORN]);
		glUniform1f(triLoc, iterationCnt);
	glUseProgram(programs[PROGRAM_BURNING_SHIP]);
		glUniform1f(shipLoc, iterationCnt);
	glUseProgram(0);
}

void set_scale() {
	static GLint mandelLoc = glGetUniformLocation(programs[PROGRAM_MANDELBROT],
	                                              "uScale");
	static GLint triLoc    = glGetUniformLocation(programs[PROGRAM_TRICORN],
	                                              "uScale");
	static GLint shipLoc   = glGetUniformLocation(programs[PROGRAM_BURNING_SHIP],
	                                              "uScale");

	// compute scene scale
	float scale = pow(10.0f, scaleExp);

	glUseProgram(programs[PROGRAM_MANDELBROT]);
		glUniform1f(mandelLoc, scale);
	glUseProgram(programs[PROGRAM_TRICORN]);
		glUniform1f(triLoc, scale);
	glUseProgram(programs[PROGRAM_BURNING_SHIP]);
		glUniform1f(shipLoc, scale);
	glUseProgram(0);
}

void set_offset() {
	static GLint mandelLoc = glGetUniformLocation(programs[PROGRAM_MANDELBROT],
	                                              "uOffset");
	static GLint triLoc    = glGetUniformLocation(programs[PROGRAM_TRICORN],
	                                              "uOffset");
	static GLint shipLoc   = glGetUniformLocation(programs[PROGRAM_BURNING_SHIP],
	                                              "uOffset");

	glUseProgram(programs[PROGRAM_MANDELBROT]);
		glUniform2f(mandelLoc, offset[0], offset[1]);
	glUseProgram(programs[PROGRAM_TRICORN]);
		glUniform2f(triLoc, offset[0], offset[1]);
	glUseProgram(programs[PROGRAM_BURNING_SHIP]);
		glUniform2f(shipLoc, offset[0], offset[1]);
	glUseProgram(0);
}

#ifdef _ANT_ENABLE

static void TW_CALL toggle_fullscreen(void *data) {
	// toggle fullscreen
	glutFullScreenToggle();
}

static void TW_CALL set_exponent_cb(const void *value, void *clientData) {
	exponent = *(const GLint *)value;
	set_exponent();
}

static void TW_CALL get_exponent_cb(void *value, void *clientData) {
	*(GLint *)value = exponent;
}

static void TW_CALL set_iteration_count_cb(const void *value,
                                           void *clientData) {
	iterationCnt = *(const GLint *)value;
	set_iteration_count();
}

static void TW_CALL get_iteration_count_cb(void *value, void *clientData) {
	*(GLint *)value = iterationCnt;
}

static void TW_CALL set_scale_cb(const void *value, void *clientData) {
	scaleExp = *(const GLfloat *)value;
	set_scale();
}

static void TW_CALL get_scale_cb(void *value, void *clientData) {
	*(GLfloat *)value = scaleExp;
}

#endif

////////////////////////////////////////////////////////////////////////////////
// on init cb
void on_init() {
	// alloc names
	buffers      = new GLuint[BUFFER_COUNT];
	vertexArrays = new GLuint[VERTEX_ARRAY_COUNT];
	programs     = new GLuint[PROGRAM_COUNT];

	// gen names
	glGenBuffers(BUFFER_COUNT, buffers);
	glGenVertexArrays(VERTEX_ARRAY_COUNT, vertexArrays);
	for(GLuint i=0; i<PROGRAM_COUNT;++i)
		programs[i] = glCreateProgram();

	// build buffers
	const GLfloat quadVertices[] = {
		-1.0f,-1.0f,0.0f,1.0f,
		 1.0f, 1.0f,0.0f,1.0f,
		-1.0f, 1.0f,0.0f,1.0f,
		 1.0f, 1.0f,0.0f,1.0f,
		-1.0f,-1.0f,0.0f,1.0f,
		 1.0f,-1.0f,0.0f,1.0f
	};
	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_QUAD_VERTICES]);
		glBufferData(GL_ARRAY_BUFFER,
		             sizeof(quadVertices),
		             quadVertices,
		             GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// build vertex arrays
	glBindVertexArray(vertexArrays[VERTEX_ARRAY_QUAD]);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_QUAD_VERTICES]);
		glVertexAttribPointer(0,4,GL_FLOAT,0,0,FW_BUFFER_OFFSET(0));
	glBindVertexArray(0);

	// configure programs
	fw::build_glsl_program(programs[PROGRAM_MANDELBROT],
	                       "fractal.glsl",
	                       "#define _MANDELBROT",
	                       GL_TRUE);
	fw::build_glsl_program(programs[PROGRAM_TRICORN],
	                       "fractal.glsl",
	                       "#define _TRICORN",
	                       GL_TRUE);
	fw::build_glsl_program(programs[PROGRAM_BURNING_SHIP],
	                       "fractal.glsl",
	                       "#define _BURNING_SHIP",
	                       GL_TRUE);
	set_scale();
	set_iteration_count();
	set_offset();
	set_exponent();

	glClearColor(0.0,0.0,0.0,1.0);

#ifdef _ANT_ENABLE
	// start ant
	TwInit(TW_OPENGL, NULL);
	// send the ''glutGetModifers'' function pointer to AntTweakBar
	TwGLUTModifiersFunc(glutGetModifiers);

	// Create a new bar
	TwBar* menuBar = TwNewBar("menu");
	TwDefine("menu size='200 600'");
	TwDefine("menu position='0 0'");
	TwDefine("menu valueswidth=85");

	TwAddVarRO(menuBar,
	           "speed (ms)",
	           TW_TYPE_FLOAT,
	           &speed,
	           "group='application'");
	TwAddButton(menuBar,
	            "fullscreen",
	            &toggle_fullscreen,
	            NULL,
	            "label='toggle fullscreen' group='application'");
	TwEnumVal fractalMode[] = {
		{PROGRAM_MANDELBROT,    "Mandelbrot"},
		{PROGRAM_TRICORN,       "Mandelbar" },
		{PROGRAM_BURNING_SHIP,  "Burning ship"}
	};
	TwType filterType= TwDefineEnum("Fractal", fractalMode, 3);
	TwAddVarRW(menuBar,
	           "set",
	           filterType,
	           &fractalProgram,
	           "help='Set the fractal set.' group='fractal'");
	TwAddVarCB(menuBar,
	           "iterations",
	           TW_TYPE_INT32,
	           &set_iteration_count_cb,
	           &get_iteration_count_cb,
	           NULL,
	           "min=1 step=1 group='fractal'");
	TwAddVarCB(menuBar,
	           "scale",
	           TW_TYPE_FLOAT,
	           &set_scale_cb,
	           &get_scale_cb,
	           NULL,
	           "step=0.1 group='fractal'");
	TwAddVarCB(menuBar,
	           "exponent",
	           TW_TYPE_INT32,
	           &set_exponent_cb,
	           &get_exponent_cb,
	           NULL,
	           " step=1 group='fractal'");


#endif // _ANT_ENABLE
	fw::check_gl_error();
}


////////////////////////////////////////////////////////////////////////////////
// on clean cb
void on_clean() {
	// delete objects
	glDeleteBuffers(BUFFER_COUNT, buffers);
	glDeleteVertexArrays(VERTEX_ARRAY_COUNT, vertexArrays);
	for(GLuint i=0; i<PROGRAM_COUNT;++i)
		glDeleteProgram(programs[i]);

	// release memory
	delete[] buffers;
	delete[] vertexArrays;
	delete[] programs;

#ifdef _ANT_ENABLE
	TwTerminate();
#endif // _ANT_ENABLE

	fw::check_gl_error();
}


////////////////////////////////////////////////////////////////////////////////
// on update cb
void on_update() {
	// Variables
	static fw::Timer deltaTimer;
	GLint windowWidth  = glutGet(GLUT_WINDOW_WIDTH);
	GLint windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
//	float aspect       = float(windowWidth)/float(windowHeight);

	// stop timing and set delta
	deltaTimer.Stop();
	deltaTicks = deltaTimer.Ticks();
#ifdef _ANT_ENABLE
	speed = deltaTicks*1000.0f;
#endif

	// set viewport
	glViewport(200,0,windowWidth, windowHeight);

	// clear back buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render
	glUseProgram(programs[fractalProgram]);
	glBindVertexArray(vertexArrays[VERTEX_ARRAY_QUAD]);
	glDrawArrays(GL_TRIANGLES,
	             0,
	             6);

	glBindVertexArray(0);

#ifdef _ANT_ENABLE
	// back to default vertex array
	TwDraw();
#endif // _ANT_ENABLE

	fw::check_gl_error();

	// start ticking
	deltaTimer.Start();

	glutSwapBuffers();
	glutPostRedisplay();
}


////////////////////////////////////////////////////////////////////////////////
// on resize cb
void on_resize(GLint w, GLint h) {
#ifdef _ANT_ENABLE
	TwWindowSize(w, h);
#endif
}


////////////////////////////////////////////////////////////////////////////////
// on key down cb
void on_key_down(GLubyte key, GLint x, GLint y) {
#ifdef _ANT_ENABLE
	if(1==TwEventKeyboardGLUT(key, x, y))
		return;
#endif
	if (key==27) // escape
		glutLeaveMainLoop();
	if(key=='f')
		glutFullScreenToggle();
	if(key=='p')
		fw::save_gl_front_buffer(0,
		                         0,
		                         glutGet(GLUT_WINDOW_WIDTH),
		                         glutGet(GLUT_WINDOW_HEIGHT));
}


////////////////////////////////////////////////////////////////////////////////
// on mouse button cb
void on_mouse_button(GLint button, GLint state, GLint x, GLint y) {
#ifdef _ANT_ENABLE
	if(1 == TwEventMouseButtonGLUT(button, state, x, y))
		return;
#endif // _ANT_ENABLE
	if(state==GLUT_DOWN) {
		mouseLeft  |= button == GLUT_LEFT_BUTTON;
		mouseRight |= button == GLUT_RIGHT_BUTTON;
	}
	else {
		mouseLeft  &= button == GLUT_LEFT_BUTTON ? false : mouseLeft;
		mouseRight  &= button == GLUT_RIGHT_BUTTON ? false : mouseRight;
	}

	if(button==3) {
		scaleExp-=0.01f;
		set_scale();
	}
	if(button==4) {
		scaleExp+=0.01f;
		set_scale();
	}
}


////////////////////////////////////////////////////////////////////////////////
// on mouse motion cb
void on_mouse_motion(GLint x, GLint y) {
#ifdef _ANT_ENABLE
	if(1 == TwEventMouseMotionGLUT(x,y))
		return;
#endif // _ANT_ENABLE

	static GLint sMousePreviousX = 0;
	static GLint sMousePreviousY = 0;
	const GLint MOUSE_XREL = x-sMousePreviousX;
	const GLint MOUSE_YREL = y-sMousePreviousY;
	sMousePreviousX = x;
	sMousePreviousY = y;

	if(mouseLeft) {
		offset += pow(10.0f, scaleExp)*0.001f*Vector2(-MOUSE_XREL,MOUSE_YREL);
		set_offset();
	}
}


////////////////////////////////////////////////////////////////////////////////
// on mouse wheel cb
void on_mouse_wheel(GLint wheel, GLint direction, GLint x, GLint y) {
#ifdef _ANT_ENABLE
	if(1 == TwMouseWheel(wheel))
		return;
#endif // _ANT_ENABLE
}


////////////////////////////////////////////////////////////////////////////////
// Main
//
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
	const GLuint CONTEXT_MAJOR = 3;
	const GLuint CONTEXT_MINOR = 3;

	// init glut
	glutInit(&argc, argv);
	glutInitContextVersion(CONTEXT_MAJOR ,CONTEXT_MINOR);
#ifdef _ANT_ENABLE
	glutInitContextFlags(GLUT_DEBUG);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
#else
	glutInitContextFlags(GLUT_DEBUG | GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
#endif

	// build window
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("fractal");

	// init glew
	glewExperimental = GL_TRUE; // segfault on GenVertexArrays on Nvidia otherwise
	GLenum err = glewInit();
	if(GLEW_OK != err)
	{
		std::stringstream ss;
		ss << err;
		std::cerr << "glewInit() gave error " << ss.str() << std::endl;
		return 1;
	}

	// glewInit generates an INVALID_ENUM error for some reason...
	glGetError();

	// set callbacks
	glutCloseFunc(&on_clean);
	glutReshapeFunc(&on_resize);
	glutDisplayFunc(&on_update);
	glutKeyboardFunc(&on_key_down);
	glutMouseFunc(&on_mouse_button);
	glutPassiveMotionFunc(&on_mouse_motion);
	glutMotionFunc(&on_mouse_motion);
	glutMouseWheelFunc(&on_mouse_wheel);

	// run
	try {
		// run demo
		on_init();
		glutMainLoop();
	}
	catch(std::exception& e) {
		std::cerr << "Fatal exception: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

