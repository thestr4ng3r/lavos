
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
#include <component/fp_controller_component.h>

#include "demo_application.h"

class Application: public DemoApplication
{
    private:
		std::string gltf_filename;

		engine::Renderer *renderer;
		engine::Material *material;

		engine::AssetContainer *asset_container;

		engine::MaterialInstance *material_instance;

		engine::Scene *scene;


		double last_cursor_x, last_cursor_y;
		engine::FirstPersonControllerComponent *fp_controller;


		void InitWindow() override;
		void InitVulkan() override;

		void RecreateSwapchain() override;

		void CleanupApplication() override;

		void Update(float delta_time) override;
		void DrawFrame(uint32_t image_index) override;

	public:
		Application(std::string gltf_filename);
};

#endif
