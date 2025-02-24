#include "cglm/cglm.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "common.h"

typedef struct ALIGN(32) Vertex {
	vec2 pos;
	vec3 color;
} Vertex;

const Vertex vertices[] = {
    {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}	
};

struct ALIGN(32) AttributeDescriptions {
	VkVertexInputAttributeDescription descriptions[2];
};

VkVertexInputBindingDescription vertexGetBindingDescription(void)
{
	VkVertexInputBindingDescription bindingDescription = {0};

	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

struct AttributeDescriptions vertexGetAttributeDescriptions(void)
{
	struct AttributeDescriptions attributeDescriptions = {0};

	attributeDescriptions.descriptions[0].binding = 0;
	attributeDescriptions.descriptions[0].location = 0;
	attributeDescriptions.descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions.descriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions.descriptions[1].binding = 0;
	attributeDescriptions.descriptions[1].location = 1;
	attributeDescriptions.descriptions[1].format =
		VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions.descriptions[1].offset = offsetof(Vertex, color);

	return attributeDescriptions;
}
