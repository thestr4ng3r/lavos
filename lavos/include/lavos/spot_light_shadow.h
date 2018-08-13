
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
		vk::DescriptorPool descriptor_pool; // TODO: can we make this more global?
		vk::DescriptorSet descriptor_set;

		void CreateImage();
		void CreateFramebuffer();
		void CreateUniformBuffer();
		void CreateDescriptorPool();
		void CreateDescriptorSet();
		void UpdateMatrixUniformBuffer();

	public:
		SpotLightShadow(Engine *engine, SpotLightComponent *light, SpotLightShadowRenderer *renderer);
		~SpotLightShadow();

		vk::CommandBuffer BuildCommandBuffer(Renderer *renderer);

		vk::Semaphore GetSemaphore()		{ return semaphore; }
};

}

#endif //LAVOS_SPOT_LIGHT_SHADOW_H
