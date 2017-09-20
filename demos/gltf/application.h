
#ifndef VULKAN_MESH_APPLICATION_H
#define VULKAN_MESH_APPLICATION_H

#include <demo_application.h>
#include <vulkan/vulkan.hpp>
#include <engine.h>
#include <vertex.h>
#include <material/material.h>
#include <material/material_instance.h>
#include <mesh.h>
#include <renderer.h>

#include <glm/glm.hpp>
#include <asset_container.h>

#include "demo_application.h"

class Application: public DemoApplication
{
    private:
		std::string gltf_filename;

		engine::Renderer *renderer;
		engine::Material *material;

		engine::AssetContainer *asset_container;

		engine::MaterialInstance *material_instance;

		engine::Node *camera_node;


		void InitVulkan() override;

		void RecreateSwapchain() override;

		void CleanupApplication() override;

		void DrawFrame(uint32_t image_index) override;

	public:
		Application(std::string gltf_filename);
};

#endif
