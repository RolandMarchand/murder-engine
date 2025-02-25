#include "glad.h"
#include "GLFW/glfw3.h"
#include "common.h"

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
	(void)width;
	(void)height;
}

Error initGraphics(void)
{
	return ERR_OK;
}

Error drawFrame(void)
{
	return ERR_OK;
}

void cleanupGraphics(void)
{
}

void frameCleanup(void)
{
}
