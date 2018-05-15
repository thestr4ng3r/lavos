
#ifndef LAVOS_VERTEX_H
#define LAVOS_VERTEX_H

#include "glm_config.h"

#include <vulkan/vulkan.hpp>

#include <iostream>

namespace lavos
{

struct alignas(sizeof(float)) Vertex
{
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 tang;
	glm::vec3 bitang;

	void SetNormalTangComputeBitang(const glm::vec3 &normal, const glm::vec4 &tang)
	{
		this->normal = normal;
		this->tang = glm::vec3(tang.x, tang.y, tang.z);
		this->bitang = glm::cross(normal, this->tang) * tang.w;
	}

	static vk::VertexInputBindingDescription GetBindingDescription()
	{
		return vk::VertexInputBindingDescription()
			.setBinding(0)
			.setStride(sizeof(Vertex))
			.setInputRate(vk::VertexInputRate::eVertex);
	}

	static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescription()
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
				.setFormat(vk::Format::eR32G32Sfloat)
				.setOffset(static_cast<uint32_t>(offsetof(Vertex, uv))),

			vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(2)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(static_cast<uint32_t>(offsetof(Vertex, normal))),

			vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(3)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(static_cast<uint32_t>(offsetof(Vertex, tang))),

			vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(4)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(static_cast<uint32_t>(offsetof(Vertex, bitang))),
		};
	};
};

}

#endif //VULKAN_VERTEX_H
