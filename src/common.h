#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "config.h"

/* CGLM requires a specific alignment for vec4 and mat4 to utilize SIMD */
#define GLM_ALLOCN(T, COUNT) _Generic((T){0},			  \
	vec4: aligned_alloc(alignof(vec4), sizeof(vec4) * COUNT), \
	mat4: aligned_alloc(alignof(mat4), sizeof(mat4) * COUNT), \
	default: malloc(sizeof(T) * COUNT))
#define GLM_ALLOC(T) GLM_ALLOCN(T, 1)

#define ALIGN(x) __attribute__((aligned(x)))

#if defined(__builtin_expect)
#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)
#else
#define unlikely(expr) (expr)
#define likely(expr) (expr)
#endif

typedef enum Error {
	ERR_OK = 0,
	ERR_INVALID_ARGUMENTS,
	ERR_SHADER_CREATION_FAILED,
	ERR_WINDOW_CREATION_FAILED,
	ERR_TEXTURE_LOADING_FAILED,

	/* OpenGL */
	ERR_GLAD_INITIALIZATION_FAILED,

	/* Vulkan */
	ERR_COMMAND_BUFFER_ALLOCATION_FAILED,
	ERR_COMMAND_BUFFER_DRAWING_FAILED,
	ERR_COMMAND_BUFFER_RECORDING_FAILED,
	ERR_COMMAND_POOL_CREATION_FAILED,
	ERR_DEBUG_MESSENGER_CREATION_FAILED,
	ERR_FRAMEBUFFER_CREATION_FAILED,
	ERR_GRAPHICS_PIPELINE_CREATION_FAILED,
	ERR_IMAGE_VIEW_CREATION_FAILED,
	ERR_INSTANCE_CREATION_FAILED,
	ERR_LOGICAL_DEVICE_CREATION_FAILED,
	ERR_NO_GPU_FOUND,
	ERR_PIPELINE_LAYOUT_CREATION_FAILED,
	ERR_RENDER_PASS_CREATION_FAILED,
	ERR_SEMAPHORE_CREATION_FAILED,
	ERR_SWAP_CHAIN_CREATION_FAILED,
	ERR_SWAP_CHAIN_PRESENTATION_FAILED,
	ERR_WINDOW_SURFACE_CREATION_FAILED,
} Error;

enum: int {
	WIDTH = 800,
	HEIGHT = 600,
};

/* std_ds.h string hashmap */
typedef struct ALIGN(16) {
	char *key, *value;
} *Arguments;

extern Arguments arguments; /* stb_ds.h string hashmap */

void printError(Error err)
{
	static char *errorMessages[] = {
		[ERR_OK]
		= "no error found",
		[ERR_INVALID_ARGUMENTS]
		= "invalid arguments",
		[ERR_SHADER_CREATION_FAILED]
		= "shader creation failed",
		[ERR_WINDOW_CREATION_FAILED]
		= "window creation failed",
		[ERR_TEXTURE_LOADING_FAILED]
		= "texture loading failed",

		/* OpenGL */
		[ERR_GLAD_INITIALIZATION_FAILED]
		= "GLAD initialization failed",
		
		/* Vulkan */
		[ERR_COMMAND_BUFFER_ALLOCATION_FAILED]
		= "command buffer allocation failed",
		[ERR_COMMAND_BUFFER_DRAWING_FAILED]
		= "command buffer drawing failed",
		[ERR_COMMAND_BUFFER_RECORDING_FAILED]
		= "command buffer recording failed",
		[ERR_COMMAND_POOL_CREATION_FAILED]
		= "command pool creation failed",
		[ERR_DEBUG_MESSENGER_CREATION_FAILED]
		= "debug messenger creation failed",
		[ERR_FRAMEBUFFER_CREATION_FAILED]
		= "framebuffer creation failed",
		[ERR_GRAPHICS_PIPELINE_CREATION_FAILED]
		= "graphics pipeline creation failed",
		[ERR_IMAGE_VIEW_CREATION_FAILED]
		= "image view creation failed",
		[ERR_INSTANCE_CREATION_FAILED]
		= "instance creation failed",
		[ERR_LOGICAL_DEVICE_CREATION_FAILED]
		= "logical device creation failed",
		[ERR_NO_GPU_FOUND]
		= "no GPU found",
		[ERR_PIPELINE_LAYOUT_CREATION_FAILED]
		= "pipeline layout creation failed",
		[ERR_RENDER_PASS_CREATION_FAILED]
		= "render pass creation failed",
		[ERR_SEMAPHORE_CREATION_FAILED]
		= "semaphore creation failed",
		[ERR_SWAP_CHAIN_CREATION_FAILED]
		= "swap chain creation failed",
		[ERR_SWAP_CHAIN_PRESENTATION_FAILED]
		= "swap chain presentation failed",
		[ERR_WINDOW_SURFACE_CREATION_FAILED]
		= "window surface creation failed"
	};

	(void)fprintf(stderr, "Error: %s\n", errorMessages[err]);
}
