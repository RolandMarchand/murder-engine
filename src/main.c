#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "stb_ds.h"

#ifdef VULKAN_ENABLED
#include "vulkan.c"
#else /* !VULKAN_ENABLED */
#include "opengl.c"
#endif /* VULKAN_ENABLED */

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
	sh_new_arena(arguments);
	for (int i = 0; i < argc; i++) {
		len = strlen(argv[i]);
		/* if odd */
		if (i & 1) {
			shput(arguments, option, strdup(argv[i]));
			continue;
		}
		if (unlikely(len < 3)) {
			*error = "expected option: " USAGE;
			return;
		}
		if (unlikely(argv[i][0] != '-' || argv[i][1] != '-')) {
			*error = "unknown option: " USAGE;
			return;
		}
		option = &argv[i][2];
	}
#undef USAGE
}


Error init()
{
	Error e = ERR_OK;

	e = initWindow();
	if (e == ERR_OK) {
		e = initGraphics();
	}

	return e;
}

void mainLoop(void)
{
	Error e = ERR_OK;
	do {
		glfwPollEvents();
		e = drawFrame();
		if (e != ERR_OK) {
			return;
		}
	} while (!glfwWindowShouldClose(window));
	frameCleanup();
}

void freeArguments(void)
{
	for (int i = 0; i < shlen(arguments); i++) {
		free(arguments[i].value);
	}
	shfree(arguments);
}

void cleanup(void)
{
	cleanupGraphics();
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

	Error e = init();
	if (e != ERR_OK) {
		return e;
	}

	mainLoop();

	cleanup();
}
