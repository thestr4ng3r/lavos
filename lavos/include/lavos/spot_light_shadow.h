
#ifndef LAVOS_SPOT_LIGHT_SHADOW_H
#define LAVOS_SPOT_LIGHT_SHADOW_H

#include "image.h"
#include "buffer.h"

namespace lavos
{

class SpotLightComponent;
class Renderer;

class SpotLightShadow
{
	private:
		Engine * const engine;
		SpotLightComponent * const light;

		std::uint32_t width;
		std::uint32_t height;
		vk::Format format;
		vk::Filter mag_filter;
		vk::Filter min_filter;

		Image image;
		vk::ImageView image_view;
		vk::Sampler sampler;

		vk::Framebuffer framebuffer;
		vk::RenderPass render_pass;
		vk::CommandBuffer command_buffer;
		vk::Semaphore semaphore;

		lavos::Buffer matrix_uniform_buffer;

		void CreateImage();
		void CreateRenderPass();
		void CreateFramebuffer();
		void CreateUniformBuffer();

	public:
		SpotLightShadow(Engine *engine, SpotLightComponent *light, std::uint32_t width, std::uint32_t height);
		~SpotLightShadow();

		void BuildCommandBuffer(Renderer *renderer);
};

}

#endif //LAVOS_SPOT_LIGHT_SHADOW_H
