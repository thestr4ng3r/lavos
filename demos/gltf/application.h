
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

class Application
{
    private:
		lavosframe::WindowApplication app;

		std::string gltf_filename;

		lavos::Renderer *renderer;
		lavos::Material *material;

		lavos::AssetContainer *asset_container;

		lavos::MaterialInstance *material_instance;

	public:
		Application(std::string gltf_filename);
		~Application();

		void Run();
};

#endif
