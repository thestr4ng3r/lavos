
#ifndef LAVOS_LAVOS_RENDERER_H
#define LAVOS_LAVOS_RENDERER_H

#include <lavos_window.h>

class LavosWindowRenderer : public lavos::shell::qt::LavosWindow::Renderer
{
	private:
		lavos::Engine *engine = nullptr;

		lavos::RenderConfig render_config;

		lavos::PhongMaterial *material = nullptr;
		lavos::Scene *scene = nullptr;
		lavos::CameraComponent *camera = nullptr;

		lavos::Renderer *renderer = nullptr;

		lavos::AssetContainer *asset_container = nullptr;

	public:
		LavosWindowRenderer(lavos::Engine *engine);
		~LavosWindowRenderer();

		void InitializeSwapchainResources(lavos::shell::qt::LavosWindow *window) override;
		void ReleaseSwapchainResources() override;

		void Render(lavos::shell::qt::LavosWindow *window);
};

#endif //LAVOS_LAVOS_RENDERER_H
