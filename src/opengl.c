#include "GLFW/glfw3.h"
#include "common.h"

void openglFramebufferResizeCallback(GLFWwindow *window, int width, int height)
{
	(void)window;
	(void)width;
	(void)height;
}

err initOpengl(void)
{
	return ERR_OK;
}

err drawFrame(void)
{
	return ERR_OK;
}

void cleanupOpengl(void)
{
}
