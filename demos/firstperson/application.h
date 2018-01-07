
#ifndef VULKAN_MESH_APPLICATION_H
#define VULKAN_MESH_APPLICATION_H

#include <vulkan/vulkan.hpp>
#include <engine.h>
#include <vertex.h>
#include <material/material.h>
#include <material/material_instance.h>
#include <mesh.h>
#include <renderer.h>

#include <window_application.h>

#include <glm/glm.hpp>
#include <asset_container.h>
#include <component/fp_controller_component.h>

class Application
{
    private:
		lavosframe::WindowApplication app;

		std::string gltf_filename;

		lavos::Renderer *renderer;
		lavos::Material *material;

		lavos::AssetContainer *asset_container;

		lavos::MaterialInstance *material_instance;

		lavos::Scene *scene;

		double last_cursor_x, last_cursor_y;
		lavos::FirstPersonControllerComponent *fp_controller;
	public:
		Application(std::string gltf_filename);
		~Application();

		void Run();
		void Update(float delta_file);
};

#endif
