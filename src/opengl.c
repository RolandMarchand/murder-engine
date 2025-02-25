#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "common.h"

#include "glad/glad.c"

GLFWwindow *window;

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

	return ERR_OK;
}

Error initGraphics(void)
{
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
	return ERR_OK;
}

void frameCleanup(void)
{
}

void mainLoop(void)
{
	Error e = ERR_OK;
	do {
		glfwSwapBuffers(window);
		glfwPollEvents();
		e = drawFrame();
		if (e != ERR_OK) {
			printError(e);
			return;
		}
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
