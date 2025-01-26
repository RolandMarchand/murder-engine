#include <stdio.h>
#include <string.h>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "cglm/cglm.h"
#include "common.h"
#include "stb_ds.h"

#define WIDTH 800
#define HEIGHT 600

GLFWwindow* window;

#include "vulkan.c"

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	(void)mods;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

err initWindow(void)
{
	if (!glfwInit()) {
		return 1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
	if (!window) {
		return 2;
	}

	glfwSetKeyCallback(window, keyCallback);

	return 0;
}

void mainLoop(void)
{
	do {
		glfwPollEvents();
	} while (!glfwWindowShouldClose(window));
}

void cleanup(void)
{
	cleanupVulkan();
	glfwDestroyWindow(window);
	glfwTerminate();
}

int main(void)
{
	err e = ERR_OK;

	e = initWindow();
	if (e) {
		return e;
	}

	e = initVulkan();
	if (e) {
		return e;
	}

	mainLoop();

	cleanup();
}
