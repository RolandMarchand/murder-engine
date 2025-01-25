#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "stb_ds.h"

#ifdef NDEBUG
#define ENABLE_VALIDATION_LAYERS false
#else
#define ENABLE_VALIDATION_LAYERS true
#endif

typedef int err;
/* Must initialize to NULL. To use with stb_ds. */
typedef const char **vector_str;

VkInstance instance;
VkDebugUtilsMessengerEXT debugMessenger;

const char *const validationLayers[] = {
	"VK_LAYER_KHRONOS_validation",
};

bool checkValidationLayerSupport(void) {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);
	VkLayerProperties *availableLayers =
		malloc(layerCount * sizeof(VkLayerProperties));
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

	bool layersAllFound = true;
	for (size_t i = 0; i < sizeof(validationLayers) / sizeof(char*); i++) {
		bool layerFound = false;
		const char *layerName = validationLayers[i];

		for (uint32_t j = 0; j < layerCount; j++) {
			VkLayerProperties layerProperties =
				availableLayers[j];
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			layersAllFound = false;
			break;
		}
	}

	free(availableLayers);
	return layersAllFound ;
}

/* The caller is responsible for calling `arrfree()` on the returned vector*/
vector_str getRequiredExtensions(void) {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	/* Freed automatically by GLFW. */
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	vector_str extensions = NULL;
	arrsetcap(extensions, glfwExtensionCount + 2);
	for (uint32_t i = 0; i < glfwExtensionCount; i++) {
		arrput(extensions, glfwExtensions[i]);
	}

	if (ENABLE_VALIDATION_LAYERS) {
		arrput(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *pUserData)
{
	(void)messageType;
	(void)pUserData;

	char *severity;
	switch(messageSeverity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		/* Not worth reporting a message. */
		return VK_FALSE;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		severity = "Warning";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
	default:
		severity = "Error";
	}

	char *type;
	switch(messageType) {
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		type = "[general] ";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		type = "[validation] ";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		type = "[performance] ";
		break;
	default:
		type = "";
	}

	fprintf(stderr, "%s: %s%s\n", severity, type, pCallbackData->pMessage);
	return VK_FALSE;
}

void populateDebugMessengerCreateInfo(
	VkDebugUtilsMessengerCreateInfoEXT *createInfo)
{
	memset(createInfo, 0, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
	createInfo->sType =
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo->messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo->messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo->pfnUserCallback = debugCallback;
}

VkResult createInstance(void)
{
	if (ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
		return 10;
	}

	VkApplicationInfo appInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "hello triangle",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0,
		.pNext = NULL
	};

	VkInstanceCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {0};
	vector_str extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = arrlen(extensions);
        createInfo.ppEnabledExtensionNames = extensions;
        if (ENABLE_VALIDATION_LAYERS) {
		createInfo.enabledLayerCount =
			sizeof(validationLayers) / sizeof(char*);
		createInfo.ppEnabledLayerNames = validationLayers;
		populateDebugMessengerCreateInfo(&debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = NULL;
	}

	VkResult result = vkCreateInstance(&createInfo, NULL, &instance);
	arrfree(extensions);

	return result == VK_SUCCESS ? 0 : 311;
}

VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
	const VkAllocationCallbacks *pAllocator,
	VkDebugUtilsMessengerEXT *pDebugMessenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT func =
		(PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
			instance,
			"vkCreateDebugUtilsMessengerEXT");
	if (func != NULL) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

err setupDebugMessenger(void)
{
	if (!ENABLE_VALIDATION_LAYERS) {
		return 0;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(&createInfo);

	VkResult res = CreateDebugUtilsMessengerEXT(instance, &createInfo,
						    NULL, &debugMessenger);
	if (res != VK_SUCCESS) {
		return 321;
	} else {
		return 0;
	}
}

err initVulkan(void)
{
	if (!glfwVulkanSupported()) {
		return 3;
	}

	err e = createInstance();
	if (e) {
		return e;
	}

	e = setupDebugMessenger();
	if (e) {
		/* Failed to setup a debug messenger. */
		return e;
	}
	return 0;
}

void DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks *pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func =
		(PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
			instance,
			"vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL) {
		func(instance, debugMessenger, pAllocator);
	}
}

void cleanupVulkan(void)
{
	if (ENABLE_VALIDATION_LAYERS) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
	}
	vkDestroyInstance(instance, NULL);
}
