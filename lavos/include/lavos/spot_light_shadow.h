
#ifndef LAVOS_SPOT_LIGHT_SHADOW_H
#define LAVOS_SPOT_LIGHT_SHADOW_H

#include "image.h"
#include "buffer.h"

#include "glm_config.h"
#include <glm/ext/matrix_float4x4.hpp>

namespace lavos
{

class SpotLight;
class Renderer;
class SpotLightShadowRenderer;

class SpotLightShadow
{
	private:
		Engine * const engine;
		SpotLight * const light;
		SpotLightShadowRenderer * const renderer;

		float near_clip;
		float far_clip;

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

		glm::mat4 GetModelViewMatrix();
		glm::mat4 GetProjectionMatrix();

		void CreateImage();
		void CreateFramebuffer();
		void CreateUniformBuffer();
		void CreateDescriptorPool();
		void CreateDescriptorSet();
		void UpdateMatrixUniformBuffer();

	public:
		SpotLightShadow(Engine *engine, SpotLight *light, SpotLightShadowRenderer *renderer, float near_clip, float far_clip);
		~SpotLightShadow();

		vk::CommandBuffer BuildCommandBuffer(Renderer *renderer);

		vk::Semaphore GetSemaphore()		{ return semaphore; }

		glm::mat4 GetModelViewProjectionMatrix()	{ return GetProjectionMatrix() * GetModelViewMatrix(); }
};

}

#endif //LAVOS_SPOT_LIGHT_SHADOW_H
