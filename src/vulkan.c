#include <assert.h>
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

#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/* Must initialize to nullptr. To use with stb_ds. */
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

typedef struct {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR formats[128];
	VkPresentModeKHR presentModes[7];
	uint32_t formatsLength;
	uint32_t presentModesLength;
} ALIGN(128) SwapChainSupportDetails;

/* stb_ds.h string hashmap */
extern Arguments arguments;

VkInstance instance;
VkDebugUtilsMessengerEXT debugMessenger;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;
VkSurfaceKHR surface;
VkSwapchainKHR swapChain;
VkImage *swapChainImages; /* stb_ds.h array */
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkImageView *swapChainImageViews; /* stb_ds.h array */
VkPipelineLayout pipelineLayout;
VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;

extern GLFWwindow *window;

const char *const validationLayers[] = {
	"VK_LAYER_KHRONOS_validation",
};
const char *const deviceExtensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

bool checkValidationLayerSupport(void)
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	VkLayerProperties *availableLayers =
		malloc(layerCount * sizeof(VkLayerProperties));
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

	bool layersAllFound = true;
	for (size_t i = 0; i < sizeof(validationLayers) / sizeof(char *); i++) {
		bool layerFound = false;
		const char *layerName = validationLayers[i];

		for (uint32_t j = 0; j < layerCount; j++) {
			VkLayerProperties layerProperties = availableLayers[j];
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
	return layersAllFound;
}

/* The caller is responsible for calling `arrfree()` on the returned vector*/
vector_str getRequiredExtensions(void)
{
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions = nullptr;
	/* Freed automatically by GLFW. */
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	vector_str extensions = nullptr;
	arrsetcap(extensions, glfwExtensionCount);
	for (uint32_t i = 0; i < glfwExtensionCount; i++) {
		arrput(extensions, glfwExtensions[i]);
	}

	if (ENABLE_VALIDATION_LAYERS) {
		arrput(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	      VkDebugUtilsMessageTypeFlagsEXT messageType,
	      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	      void *pUserData)
{
	(void)pUserData;

	char *severity = "";
	switch (messageSeverity) {
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
	switch (messageType) {
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
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo->messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo->pfnUserCallback = debugCallback;
}

VkResult createInstance(void)
{
	if (ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
		return 10;
	}

	VkApplicationInfo appInfo = { 0 };
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "hello triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;
	appInfo.pNext = nullptr;

	VkInstanceCreateInfo createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = { 0 };
	vector_str extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = arrlen(extensions);
	createInfo.ppEnabledExtensionNames = extensions;
	if (ENABLE_VALIDATION_LAYERS) {
		createInfo.enabledLayerCount =
			sizeof(validationLayers) / sizeof(char *);
		createInfo.ppEnabledLayerNames = validationLayers;
		populateDebugMessengerCreateInfo(&debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
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
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
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
						    nullptr, &debugMessenger);
	if (res != VK_SUCCESS) {
		return 321;
	}
	return ERR_OK;
}

void destroyDebugUtilsMessengerEXT(VkInstance instance,
				   VkDebugUtilsMessengerEXT debugMessenger,
				   const VkAllocationCallbacks *pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void cleanupVulkan(void)
{
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	for (int i = 0; i < arrlen(swapChainImageViews); i++) {
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(device, swapChain, nullptr);
	vkDestroyDevice(device, nullptr);
	if (ENABLE_VALIDATION_LAYERS) {
		destroyDebugUtilsMessengerEXT(instance, debugMessenger,
					      nullptr);
	}
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	arrfree(swapChainImages);
	arrfree(swapChainImageViews);
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
						 nullptr);
	VkQueueFamilyProperties *queueFamilies = nullptr;
	arrsetlen(queueFamilies, queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
						 queueFamilies);

	QueueFamilyIndices indices = { 0 };

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

	arrfree(queueFamilies);

	return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount = 0;
	VkExtensionProperties *availableExtensions = nullptr;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
					     nullptr);
	arrsetlen(availableExtensions, extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
					     availableExtensions);

	size_t deviceExtensionsCount =
		sizeof(deviceExtensions) / sizeof(char *);
	struct ALIGN(16) {
		char *key;
		bool value;
	} *requiredExtensions = nullptr;
	for (size_t i = 0; i < deviceExtensionsCount; i++) {
		shput(requiredExtensions, deviceExtensions[i], true);
	}

	for (uint32_t i = 0; i < extensionCount; i++) {
		(void)shdel(requiredExtensions,
			    availableExtensions[i].extensionName);
	}

	bool supported = shlen(requiredExtensions) == 0;

	shfree(requiredExtensions);
	arrfree(availableExtensions);

	return supported;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR *capabilities)
{
	if (capabilities->currentExtent.width != UINT32_MAX) {
		return capabilities->currentExtent;
	}

	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window, &width, &height);

	VkExtent2D actualExtent = {
		(uint32_t)width,
		(uint32_t)height,
	};

	actualExtent.width = CLAMP(actualExtent.width,
				   capabilities->minImageExtent.width,
				   capabilities->maxImageExtent.width);
	actualExtent.height = CLAMP(actualExtent.height,
				    capabilities->minImageExtent.height,
				    capabilities->maxImageExtent.height);

	return actualExtent;
}

/* Return VK_PRESENT_MODE_MAILBOX_KHR if present, otherwise
 * VK_PRESENT_MODE_FIFO_KHR. */
VkPresentModeKHR
chooseSwapPresentMode(const VkPresentModeKHR *availablePresentModes,
		      uint32_t availablePresentModesLength) /* must be >= 1 */
{
	assert(availablePresentModesLength >= 1);

	for (uint32_t i = 0; i < availablePresentModesLength; i++) {
		VkPresentModeKHR availablePresentMode =
			availablePresentModes[i];
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR
chooseSwapSurfaceFormat(const VkSurfaceFormatKHR *availableFormats,
			uint32_t availableFormatsLength) /* must be >= 1 */
{
	assert(availableFormatsLength >= 1);

	for (uint32_t i = 0; i < availableFormatsLength; i++) {
		VkSurfaceFormatKHR availableFormat = availableFormats[i];
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
		    availableFormat.colorSpace ==
			    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details = { 0 };
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
						  &details.capabilities);

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
					     &details.formatsLength, nullptr);

	/* If SwapChainSupportDetails can't fit all formats found, truncate. */
	if (unlikely(details.formatsLength >
		     sizeof(details.formats) / sizeof(VkSurfaceFormatKHR))) {
		VkSurfaceFormatKHR *hugeFormats = calloc(
			details.formatsLength, sizeof(VkSurfaceFormatKHR));
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			device, surface, &details.formatsLength, hugeFormats);
		memcpy(details.formats, hugeFormats, sizeof(details.formats));
		free(hugeFormats);
	} else {
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
						     &details.formatsLength,
						     details.formats);
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(
		device, surface, &details.presentModesLength, nullptr);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
						  &details.presentModesLength,
						  details.presentModes);

	return details;
}

bool isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);
	bool indicesComplete = indices.presentFamily.exists &&
			       indices.graphicsFamily.exists;
	bool extensionsSupported = checkDeviceExtensionSupport(device);
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport =
			querySwapChainSupport(device);
		swapChainAdequate = swapChainSupport.formatsLength > 0 &&
				    swapChainSupport.presentModesLength > 0;
	}
	return indicesComplete && extensionsSupported && swapChainAdequate;
}

bool isDeviceDiscreteGPU(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	return deviceProperties.deviceType ==
		       VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
	       isDeviceSuitable(device);
}

err pickPhysicalDevice(void)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	/* No GPU found. */
	if (deviceCount == 0) {
		return 101;
	}

	VkPhysicalDevice *devices = nullptr;
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

err createSwapChain(void)
{
	SwapChainSupportDetails swapChainSupport =
		querySwapChainSupport(physicalDevice);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(
		swapChainSupport.formats, swapChainSupport.formatsLength);
	VkPresentModeKHR presentMode =
		chooseSwapPresentMode(swapChainSupport.presentModes,
				      swapChainSupport.presentModesLength);
	VkExtent2D extent = chooseSwapExtent(&swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 &&
	    imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.preTransform =
		swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value,
					  indices.presentFamily.value };

	if (indices.graphicsFamily.value != indices.presentFamily.value) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; /* Optional */
		createInfo.pQueueFamilyIndices = nullptr; /* Optional */
	}

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) !=
	    VK_SUCCESS) {
		return 171;
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	arrsetlen(swapChainImages, imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount,
				swapChainImages);
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	return ERR_OK;
}

err createLogicalDevice(void)
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	/* Graphics queue, and maybe present queue if they're the same. */
	VkDeviceQueueCreateInfo queuesCreateInfo[2] = { 0 };
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

	VkPhysicalDeviceFeatures deviceFeatures = { 0 };

	VkDeviceCreateInfo createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queuesCreateInfo;
	createInfo.queueCreateInfoCount = queuesCreateInfoLength;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount =
		sizeof(deviceExtensions) / sizeof(char *);
	createInfo.ppEnabledExtensionNames = deviceExtensions;

	if (ENABLE_VALIDATION_LAYERS) {
		createInfo.enabledLayerCount =
			sizeof(validationLayers) / sizeof(char *);
		createInfo.ppEnabledLayerNames = validationLayers;
	} else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) !=
	    VK_SUCCESS) {
		return 55;
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value, 0,
			 &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value, 0, &presentQueue);

	return ERR_OK;
}

err createSurface(void)
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) !=
	    VK_SUCCESS) {
		return 49;
	}

	return ERR_OK;
}

err createImageViews(void)
{
	int imageslength = arrlen(swapChainImages);
	arrsetlen(swapChainImageViews, imageslength);
	for (int i = 0; i < imageslength; i++) {
		VkImageViewCreateInfo createInfo = { 0 };
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask =
			VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr,
				      &swapChainImageViews[i]) != VK_SUCCESS) {
			return 9998;
		}
	}

	return ERR_OK;
}

err createRenderPass(void)
{
	VkAttachmentDescription colorAttachment = {0};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {0};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = {0};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass)
	    != VK_SUCCESS) {
		/* Failed to create a render pass. */
		return 444;
	}

	return ERR_OK;
}

/* `code` must have padding of 3 extra bytes for uint32_t
 *  interpretation. */
err createShaderModule(const char *code, size_t length, VkShaderModule *module)
{
	VkShaderModuleCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = length;
	createInfo.pCode = (uint32_t *)code;

	if (vkCreateShaderModule(device, &createInfo, nullptr, module)
	    != VK_SUCCESS) {
		return 109;
	}

	return ERR_OK;
}

err createGraphicsPipeline(void)
{
	const char vertCode[] = {
#embed "shaders/vert.spv"
		// Buffer for 32 bit interpretation
		, '\0', '\0', '\0'
	};

	const char fragCode[] = {
#embed "shaders/frag.spv"
		// Buffer for 32 bit interpretation
		, '\0', '\0', '\0'
	};

	size_t vertLength = sizeof(vertCode) - (3 * sizeof(char));
	size_t fragLength = sizeof(fragCode) - (3 * sizeof(char));

	VkShaderModule vertShaderModule = nullptr;
	VkShaderModule fragShaderModule = nullptr;
	
	err e = createShaderModule(vertCode, vertLength, &vertShaderModule);
	if (e != ERR_OK) {
		return e;
	}
	e = createShaderModule(fragCode, fragLength, &fragShaderModule);
	if (e != ERR_OK) {
		return e;
	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
	vertShaderStageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
	fragShaderStageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	/* Vertex input */
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
	vertexInputInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	/* Input assembly */
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
	inputAssembly.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	/* Viewport and scissors */
	VkViewport viewport = {0};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {0};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = swapChainExtent;

	/* Dynamic state */
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState = {0};
	dynamicState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount =
		sizeof(dynamicStates) / sizeof(VkDynamicState);
	dynamicState.pDynamicStates = dynamicStates;

	VkPipelineViewportStateCreateInfo viewportState = {0};
	viewportState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	/* Not sure if I should add that. */
	viewportState.pViewports = &viewport;
	viewportState.pScissors = &scissor;

	/* Rasterization */
	VkPipelineRasterizationStateCreateInfo rasterizer = {0};
	rasterizer.sType =
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	/* Multisampling */
	VkPipelineMultisampleStateCreateInfo multisampling = {0};
	multisampling.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	/* Depth and stencil testing */
	/* ¯\_(ツ)_/¯ */

	/* Color bending */
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
		| VK_COLOR_COMPONENT_G_BIT
		| VK_COLOR_COMPONENT_B_BIT
		| VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {0};
	colorBlending.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	/* Pipeline Layout */
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
	pipelineLayoutInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
				   &pipelineLayout) != VK_SUCCESS) {
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		return 880;
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {0};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	VkResult vkE = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
						 &pipelineInfo, nullptr,
						 &graphicsPipeline);

	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);

	if (vkE != VK_SUCCESS) {
		/* Failed to create the graphics pipeline. */
		return 445;
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

	e = createSwapChain();
	if (e != ERR_OK) {
		return e;
	}

	e = createImageViews();
	if (e != ERR_OK) {
		return e;
	}

	e = createRenderPass();
	if (e != ERR_OK) {
		return e;
	}

	e = createGraphicsPipeline();
	if (e != ERR_OK) {
		return e;
	}

	return ERR_OK;
}
