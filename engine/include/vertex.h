
#ifndef VULKAN_VERTEX_H
#define VULKAN_VERTEX_H

#include "glm_config.h"

#include <vulkan/vulkan.hpp>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 uv;

	static vk::VertexInputBindingDescription GetBindingDescription()
	{
		return vk::VertexInputBindingDescription()
			.setBinding(0)
			.setStride(sizeof(Vertex))
			.setInputRate(vk::VertexInputRate::eVertex);
	}

	static std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescription()
	{
		return {
			vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(0)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(static_cast<uint32_t>(offsetof(Vertex, pos))),

			vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(1)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(static_cast<uint32_t>(offsetof(Vertex, color))),

			vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(2)
				.setFormat(vk::Format::eR32G32Sfloat)
				.setOffset(static_cast<uint32_t>(offsetof(Vertex, uv))),
		};
	};
};

#endif //VULKAN_VERTEX_H
