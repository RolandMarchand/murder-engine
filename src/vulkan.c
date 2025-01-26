#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "common.h"
#include "stb_ds.h"

#ifdef NDEBUG
#define ENABLE_VALIDATION_LAYERS false
#else
#define ENABLE_VALIDATION_LAYERS true
#endif

/* Must initialize to NULL. To use with stb_ds. */
typedef const char **vector_str;

typedef struct QueueFamilyIndices {
	struct {
		uint32_t value;
		bool exists;
	} ALIGN(8) graphicsFamily;
	struct {
		uint32_t value;
		bool exists;
	} ALIGN(8) presentFamily;
} ALIGN(16) QueueFamilyIndices;

VkInstance instance;
VkDebugUtilsMessengerEXT debugMessenger;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;
VkSurfaceKHR surface;

extern GLFWwindow* window;

const char *const validationLayers[] = {
	"VK_LAYER_KHRONOS_validation",
};

bool checkValidationLayerSupport(void) {
	uint32_t layerCount = 0;
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
	const char** glfwExtensions = NULL;
	/* Freed automatically by GLFW. */
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	vector_str extensions = NULL;
	arrsetcap(extensions, glfwExtensionCount);
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

	char *severity = "";
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

	char *type = "";
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
		break;
	}

	(void)fprintf(stderr, "%s: %s%s\n", severity, type,
		      pCallbackData->pMessage);

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

	VkApplicationInfo appInfo = {0};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "hello triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;
	appInfo.pNext = NULL;

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

VkResult createDebugUtilsMessengerEXT(
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
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

err setupDebugMessenger(void)
{
	if (!ENABLE_VALIDATION_LAYERS) {
		return ERR_OK;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(&createInfo);

	VkResult res = createDebugUtilsMessengerEXT(instance, &createInfo,
						    NULL, &debugMessenger);
	if (res != VK_SUCCESS) {
		return 321;
	}
	return ERR_OK;
}

void destroyDebugUtilsMessengerEXT(
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
	vkDestroyDevice(device, NULL);
	if (ENABLE_VALIDATION_LAYERS) {
		destroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
	}
	vkDestroySurfaceKHR(instance, surface, NULL);
	vkDestroyInstance(instance, NULL);
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
						 NULL);
	VkQueueFamilyProperties *queueFamilies = NULL;
	arrsetlen(queueFamilies, queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
						 queueFamilies);

	QueueFamilyIndices indices = {0};
		
	for (int i = 0; i < arrlen(queueFamilies); i++) {
		VkQueueFamilyProperties queueFamily = queueFamilies[i];
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily.value = i;
			indices.graphicsFamily.exists = true;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
							     &presentSupport);
			if (presentSupport) {
				indices.presentFamily.value = i;
				indices.presentFamily.exists = true;
			}
		}
	}

	return indices;
}

bool isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);
	return indices.presentFamily.exists && indices.graphicsFamily.exists;
}

bool isDeviceDiscreteGPU(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	return deviceProperties.deviceType ==
		VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
		&& isDeviceSuitable(device);
}

err pickPhysicalDevice(void)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
	/* No GPU found. */
	if (deviceCount == 0) {
		return 101;
	}

	VkPhysicalDevice *devices = NULL;
	arrsetlen(devices, deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

	for (int i = 0; i < arrlen(devices); i++) {
		VkPhysicalDevice device = devices[i];
		if (isDeviceDiscreteGPU(device)) {
			physicalDevice = device;
			break;
		}
	}

	/* No discrete GPU found, going for any. */
	if (physicalDevice == VK_NULL_HANDLE) {
		for (int i = 0; i < arrlen(devices); i++) {
			VkPhysicalDevice device = devices[i];
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}
	}

	arrfree(devices);

	/* No GPU found. */
	if (physicalDevice == VK_NULL_HANDLE) {
		return 199;
	}

	return ERR_OK;
}

err createLogicalDevice(void)
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	/* Graphics queue, and maybe present queue if they're the same. */
	VkDeviceQueueCreateInfo queuesCreateInfo[2] = {0};
	size_t queuesCreateInfoLength = 1;
	float queuePriority = 1.0f;
	queuesCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queuesCreateInfo[0].queueFamilyIndex = indices.graphicsFamily.value;
	queuesCreateInfo[0].queueCount = 1;
	queuesCreateInfo[0].pQueuePriorities = &queuePriority;

	/* Present queue if it is different from graphics queue. */
	if (indices.graphicsFamily.value != indices.presentFamily.value) {
		queuesCreateInfoLength = 2;
		queuesCreateInfo[1].sType =
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queuesCreateInfo[1].queueFamilyIndex =
			indices.presentFamily.value;
		queuesCreateInfo[1].queueCount = 1;
		queuesCreateInfo[1].pQueuePriorities = &queuePriority;
	}

	VkPhysicalDeviceFeatures deviceFeatures = {0};

	VkDeviceCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queuesCreateInfo;
	createInfo.queueCreateInfoCount = queuesCreateInfoLength;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = 0;

	if (ENABLE_VALIDATION_LAYERS) {
		createInfo.enabledLayerCount =
			sizeof(validationLayers) / sizeof(char*);
		createInfo.ppEnabledLayerNames = validationLayers;
	} else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device) !=
	    VK_SUCCESS) {
		return 55;
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value, 0,
			 &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value, 0,
			 &presentQueue);

	return ERR_OK;
}

err createSurface(void)
{
	if (glfwCreateWindowSurface(instance, window, NULL, &surface) !=
	    VK_SUCCESS) {
		return 49;
	}

	return ERR_OK;
}

err initVulkan(void)
{
	if (!glfwVulkanSupported()) {
		return 3;
	}

	err e = createInstance();
	if (e != ERR_OK) {
		return e;
	}

	e = setupDebugMessenger();
	if (e != ERR_OK) {
		/* Failed to setup a debug messenger. */
		return e;
	}

	e = createSurface();
	if (e != ERR_OK) {
		return e;
	}

	e = pickPhysicalDevice();
	if (e != ERR_OK) {
		return e;
	}

	e = createLogicalDevice();
	if (e != ERR_OK) {
		return e;
	}

	return ERR_OK;
}
