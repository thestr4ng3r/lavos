
#ifndef LAVOS_LAVOS_RENDERER_H
#define LAVOS_LAVOS_RENDERER_H

#include "lavos_window.h"

class LavosWindowRenderer : public LavosWindow::Renderer
{
	private:
		lavos::Engine *engine = nullptr;

		lavos::PhongMaterial *material = nullptr;
		lavos::Scene *scene = nullptr;
		lavos::CameraComponent *camera = nullptr;

		lavos::Renderer *renderer = nullptr;

		lavos::AssetContainer *asset_container = nullptr;

	public:
		LavosWindowRenderer(lavos::Engine *engine);
		~LavosWindowRenderer();

		void InitializeSwapchainResources(LavosWindow *window) override;
		void ReleaseSwapchainResources() override;

		void Render(LavosWindow *window);
};

#endif //LAVOS_LAVOS_RENDERER_H
