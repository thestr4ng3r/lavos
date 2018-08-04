
#ifndef LAVOS_SPOT_LIGHT_SHADOW_H
#define LAVOS_SPOT_LIGHT_SHADOW_H

#include "image.h"
#include "buffer.h"

namespace lavos
{

class SpotLightComponent;
class Renderer;
class SpotLightShadowRenderer;

class SpotLightShadow
{
	private:
		Engine * const engine;
		SpotLightComponent * const light;
		SpotLightShadowRenderer * const renderer;

		vk::Filter mag_filter;
		vk::Filter min_filter;

		Image image;
		vk::ImageView image_view;
		vk::Sampler sampler;

		vk::Framebuffer framebuffer;
		vk::CommandBuffer command_buffer;
		vk::Semaphore semaphore;

		lavos::Buffer *matrix_uniform_buffer = nullptr;

		void CreateImage();
		void CreateFramebuffer();
		void CreateUniformBuffer();

	public:
		SpotLightShadow(Engine *engine, SpotLightComponent *light, SpotLightShadowRenderer *renderer);
		~SpotLightShadow();

		void BuildCommandBuffer(Renderer *renderer);
};

}

#endif //LAVOS_SPOT_LIGHT_SHADOW_H
