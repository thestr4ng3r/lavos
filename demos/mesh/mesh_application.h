
#ifndef VULKAN_MESH_APPLICATION_H
#define VULKAN_MESH_APPLICATION_H

#include <demo_application.h>
#include <vulkan/vulkan.hpp>
#include <engine.h>
#include <vertex.h>
#include <material.h>
#include <material_instance.h>
#include <mesh.h>
#include <renderer.h>

#include <glm/glm.hpp>

#include "demo_application.h"

class MeshApplication: public DemoApplication
{
    private:
		vk::CommandPool command_pool;
		std::vector<vk::CommandBuffer> command_buffers;

		engine::Renderer *renderer;
		engine::Material *material;

		engine::Mesh *mesh;

		engine::MaterialInstance *material_instance;

		void InitVulkan() override;

		void RecreateSwapchain() override;
		void CleanupSwapchain() override;

		void CleanupApplication() override;

		void CreateFramebuffers();

		void CreateCommandPool();
		void CreateCommandBuffers();


		void DrawFrame(uint32_t image_index) override;
};

#endif
