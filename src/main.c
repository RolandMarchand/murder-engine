#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "cglm/cglm.h"
#include "common.h"
#include "stb_ds.h"

enum {
	WIDTH = 800,
	HEIGHT = 600
};

#include "vulkan.c"

GLFWwindow *window;
Arguments arguments; /* stb_ds.h string hashmap */
extern VkFence inFlightFence;
extern VkDevice device;

/* The program expects each pair of arguments to be under the format: --option
 * value. Those key-pairs are recorded into the global `arguments` variable,
 * which is an stb_ds.h array. Return the error into input pointer `error`, and
 * sets it to nullptr if the operation was successful. */
void storeArguments(int argc, char **argv, char **error)
{
#define USAGE "usage: laz_engine [--<option> <value>]"
	// Remove the executable from the argument list
	argc--;
	argv++;

	if (argc <= 0) {
		return;
	}

	*error = nullptr;

	if (argc % 2 != 0) {
		*error = "missing or extra argument: " USAGE;
		return;
	}

	size_t len = 0;
	char *option = nullptr;
	for (int i = 0; i < argc; i++) {
		len = strlen(argv[i]);
		// option
		if (i % 2 == 0) {
			if (len < 3) {
				*error = "expected option: " USAGE;
				return;
			}
			if (argv[i][0] != '-' || argv[i][1] != '-') {
				*error = "unknown option: " USAGE;
				return;
			}
			option = &argv[i][2];
		} else {
			shput(arguments, option, argv[i]);
		}
	}
#undef USAGE
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

err initWindow(void)
{
	if (!glfwInit()) {
		return 1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(WIDTH, HEIGHT, "Laz's Engine", nullptr,
				  nullptr);
	if (!window) {
		return 2;
	}

	glfwSetKeyCallback(window, keyCallback);

	return 0;
}

void mainLoop(void)
{
	err e = ERR_OK;
	do {
		glfwPollEvents();
		e = drawFrame();
		if (e != ERR_OK) {
			return;
		}
	} while (!glfwWindowShouldClose(window));
	vkDeviceWaitIdle(device);
}

void cleanup(void)
{
	cleanupVulkan();
	glfwDestroyWindow(window);
	glfwTerminate();
}

int main(int argc, char **argv)
{
	err e = ERR_OK;

	char *argError = nullptr;
	storeArguments(argc, argv, &argError);
	if (argError != nullptr) {
		printf("%s\n", argError);
		return 622;
	}

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
