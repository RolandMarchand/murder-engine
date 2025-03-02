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
#include "script.c"

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

	/* If odd. */
	if (argc & 1) {
		*error = "missing or extra argument: " USAGE;
		return;
	}

	size_t len = 0;
	char *option = nullptr;
	sh_new_arena(arguments);
	for (int i = 0; i < argc; i++) {
		len = strlen(argv[i]);
		/* If odd. */
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

void freeArguments(void)
{
	for (int i = 0; i < shlen(arguments); i++) {
		free(arguments[i].value);
	}
	shfree(arguments);
}

void recordTime(void)
{
	lastFrameTimeSec = currentFrameTimeSec;
	currentFrameTimeSec = (float)glfwGetTime();
	deltaTimeSec = currentFrameTimeSec - lastFrameTimeSec;
}

/* Print current FPS to stdout. Does nothing if it's been less than a second
 * since the last print. */
void printFPS(void)
{
	static double lastSecondTimeSec;
	static uint64_t lastSecondFrameCount;

	double timeElapsedSec = currentFrameTimeSec - lastSecondTimeSec;
	if (timeElapsedSec < 1.0) {
		return;
	}

	printf("FPS: %.0f\n", (double)(frameCount - lastSecondFrameCount)
	       / timeElapsedSec);

	lastSecondTimeSec = currentFrameTimeSec;
	lastSecondFrameCount = frameCount;
}

Error init(void)
{
	Error e = ERR_OK;

	e = windowInit();
	if (e != ERR_OK) {
		return e;
	}
	
	e = graphicsInit();
	if (e != ERR_OK) {
		return e;
	}

	e = scriptLoad();
	if (e != ERR_OK) {
		return e;
	}

	return e;
}

void cleanup(void)
{
	cleanupGraphics();
	cleanupWindow();

	Error e = scriptUnload();
	if (e != ERR_OK) {
		printError(e);
	}
}

void mainLoop()
{
	Error e = ERR_OK;

	do {
		recordTime();

		processInput(window);

		e = scriptUpdate();
		if (e != ERR_OK) {
			printError(e);
			return;
		}

		e = drawFrame();
		if (e != ERR_OK) {
			printError(e);
			return;
		}

		frameCount++;

		/* Print FPS every 64 frames */
		printFPS();
	} while (!glfwWindowShouldClose(window));
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
		printError(e);
		return e;
	}

	mainLoop();

	cleanup();
}
