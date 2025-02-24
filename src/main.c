#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "stb_ds.h"

#ifdef VULKAN_ENABLED
#define GLFW_INCLUDE_VULKAN
#endif /* VULKAN_ENABLED */

#include "GLFW/glfw3.h"

enum: int {
	WIDTH = 800,
	HEIGHT = 600
};

#ifdef VULKAN_ENABLED
#include "vulkan.c"
extern VkDevice device;
extern bool framebufferResized;
#else /* !VULKAN_ENABLED */
#include "opengl.c"
#endif /* VULKAN_ENABLED */

GLFWwindow *window;
Arguments arguments; /* stb_ds.h string hashmap */

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

void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
#ifdef VULKAN_ENABLED
	vulkanFramebufferResizeCallback(window, width, height);
#else /* !VULKAN_ENABLED */
	openglFramebufferResizeCallback(window, width, height);
#endif /* VULKAN_ENABLED */
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
	window = glfwCreateWindow(WIDTH, HEIGHT, "Laz's Engine", nullptr,
				  nullptr);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

	if (!window) {
		return 2;
	}

	glfwSetKeyCallback(window, keyCallback);

	return 0;
}

err init()
{
	err e = ERR_OK;

	e = initWindow();
	if (e == ERR_OK) {
#ifdef VULKAN_ENABLED
		e = initVulkan();
#else /* !VULKAN_ENABLED */
		e = initOpengl();
#endif /* VULKAN_ENABLED */
	}

	return e;
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
#ifdef VULKAN_ENABLED
	vkDeviceWaitIdle(device);
#endif /* VULKAN-ENABLED */
}

void cleanup(void)
{
#ifdef VULKAN_ENABLED
	cleanupVulkan();
#else /* !VULKAN-ENABLED */
	cleanupOpengl();
#endif /* VULKAN-ENABLED */
	glfwDestroyWindow(window);
	glfwTerminate();
}

int main(int argc, char **argv)
{
	char *argError = nullptr;
	storeArguments(argc, argv, &argError);
	if (argError != nullptr) {
		printf("%s\n", argError);
		return 622;
	}

	err e = init();
	if (e != ERR_OK) {
		return e;
	}

	mainLoop();

	cleanup();
}
