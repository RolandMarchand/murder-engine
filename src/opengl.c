#include <time.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "common.h"

#include "glad/glad.c"

enum {
	INFO_LOG_SIZE = 512,
};

GLchar infoLog[INFO_LOG_SIZE];
GLFWwindow *window;
GLuint triangleVBO;
GLuint triangleVAO;
GLuint triangleEBO;
GLuint shaderProgram;

uint32_t frameCount;
double lastFrameTimeSec;
double currentFrameTimeSec;
double deltaTimeSec;

GLvoid setUniformBool(GLuint shaderID, const GLchar *name, GLboolean value)
{
	glUniform1i(glGetUniformLocation(shaderID, name), (GLint)value);
}

GLvoid setUniformInt(GLuint shaderID, const GLchar *name, GLint value)
{
	glUniform1i(glGetUniformLocation(shaderID, name), value);
}

GLvoid setUniformFloat(GLuint shaderID, const GLchar *name, GLfloat value)
{
	glUniform1f(glGetUniformLocation(shaderID, name), value);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
		 int mods)
{
	(void)scancode;
	(void)mods;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}

Error initWindow(void)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(WIDTH, HEIGHT, ENGINE_NAME, nullptr, nullptr);
	if (window == NULL) {
		glfwTerminate();
		return ERR_WINDOW_CREATION_FAILED;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		return ERR_GLAD_INITIALIZATION_FAILED;
	}

	glViewport(0, 0, WIDTH, HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSwapInterval(0);

	return ERR_OK;
}

Error compileVertexShader(GLuint *vertexShaderOut, const GLchar *code)
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &code, nullptr);
	glCompileShader(vertexShader);

	/* Shader compilation results. */
	GLint success = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, INFO_LOG_SIZE, nullptr,
				   infoLog);
		(void)fprintf(stderr, "%s\n", infoLog);

	}

	*vertexShaderOut = vertexShader;

	return ERR_OK;
}

Error compileFragmentShader(GLuint *fragmentShaderOut, const GLchar *code)
{
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &code, nullptr);
	glCompileShader(fragmentShader);

	/* Shader compilation results. */
	GLint success = 0;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, INFO_LOG_SIZE, nullptr,
				   infoLog);
		(void)fprintf(stderr, "%s\n", infoLog);
		return ERR_SHADER_CREATION_FAILED;
	}

	*fragmentShaderOut = fragmentShader;

	return ERR_OK;
}

Error linkShaderProgram(GLuint *shaderProgramOut, GLuint vertexShader,
			    GLuint fragmentShader)
{
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	/* Shader program linking results. */
	GLint success = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shaderProgram, INFO_LOG_SIZE, nullptr,
				   infoLog);
		(void)fprintf(stderr, "%s\n", infoLog);
		return ERR_SHADER_CREATION_FAILED;
	}

	*shaderProgramOut = shaderProgram;
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return ERR_OK;
}

Error compileShaderProgram(GLuint *shaderIDOut,
			   const GLchar *vertexShaderSource,
			   const GLchar *fragmentShaderSource)
{
	/* Create vertex shader. */
	GLuint vertexShader = 0;
	Error e = compileVertexShader(&vertexShader, vertexShaderSource);
	if (e != ERR_OK) {
		return e;
	}

	/* Create fragment shader. */
	GLuint fragmentShader = 0;
	e = compileFragmentShader(&fragmentShader, fragmentShaderSource);
	if (e != ERR_OK) {
		return e;
	}

	GLuint shaderProgram = 0;
	e = linkShaderProgram(&shaderProgram, vertexShader, fragmentShader);
	if (e != ERR_OK) {
		return e;
	}

	*shaderIDOut = shaderProgram;

	return ERR_OK;
}

void vertexBufferInit(void)
{
	GLfloat vertices[] = {
		0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
	}; 
	/* GLuint indices[] = { */
	/* 	0, 1, 3, */
	/* 	1, 2, 3 */
	/* }; */

	/* VAO */
	glGenVertexArrays(1, &triangleVAO);
	glBindVertexArray(triangleVAO);

	/* VBO */
	glGenBuffers(1, &triangleVBO);
	glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
		     GL_STATIC_DRAW);

	/* EBO */
	/* glGenBuffers(1, &triangleEBO); */
	/* glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleEBO); */
	/* glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, */
	/* 					     GL_STATIC_DRAW); */

	/* Set position attribute. */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
			      nullptr);
	glEnableVertexAttribArray(0);
	/* Set color attribute. */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
			      (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	

	glBindVertexArray(triangleVAO);
}

Error initGraphics(void)
{
	static const GLchar vertexShaderSource[] = {
#embed "shaders/gl-vertex.glsl"
		, '\0'
	};
	static const GLchar fragmentShaderSource[] = {
#embed "shaders/gl-fragment.glsl"
		, '\0'
	};

	Error e = compileShaderProgram(&shaderProgram, vertexShaderSource,
				       fragmentShaderSource);
	if (e != ERR_OK) {
		return e;
	}

	vertexBufferInit();

	return ERR_OK;
}

Error init(void)
{
	Error e = ERR_OK;

	e = initWindow();
	if (e == ERR_OK) {
		e = initGraphics();
	}

	if (e != ERR_OK) {
		printError(e);
	}

	return e;
}

Error drawFrame(void)
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	return ERR_OK;
}

void frameCleanup(void)
{
}

void recordTime(void)
{
	static struct timespec ts;
	if (timespec_get(&ts, TIME_UTC) == 0) {
		return;
	}
	lastFrameTimeSec = currentFrameTimeSec;
	currentFrameTimeSec =
		(double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
	deltaTimeSec = currentFrameTimeSec - lastFrameTimeSec;
}

/* Print current FPS to stdout. Does nothing if it's been less than a second
 * since the last print. */
void printFPS(void)
{
	static double lastSecondTimeSec;
	static uint32_t lastSecondFrameCount;

	double timeElapsedSec = currentFrameTimeSec - lastSecondTimeSec;
	if (currentFrameTimeSec - lastSecondTimeSec < 1.0) {
		return;
	}

	printf("FPS: %.0f\n", (frameCount - lastSecondFrameCount)
	       / timeElapsedSec);

	lastSecondTimeSec = currentFrameTimeSec;
	lastSecondFrameCount = frameCount;
}

void mainLoop(void)
{
	Error e = ERR_OK;

	do {
		recordTime();

		glfwPollEvents();

		e = drawFrame();
		if (e != ERR_OK) {
			printError(e);
			return;
		}

		glfwSwapBuffers(window);
		
		frameCount++;

		/* Print FPS every 64 frames */
		printFPS();
	} while (!glfwWindowShouldClose(window));
	frameCleanup();
}

void cleanupGraphics(void)
{
}

void cleanup(void)
{
	cleanupGraphics();
	glfwDestroyWindow(window);
	glfwTerminate();
}
