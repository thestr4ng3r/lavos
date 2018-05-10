
#ifndef LAVOS_SWAPCHAIN_H
#define LAVOS_SWAPCHAIN_H

#include "render_target.h"

namespace lavos
{

class Swapchain : public ColorRenderTarget
{
	private:
		Engine * const engine;

		vk::SurfaceKHR surface;

		vk::SurfaceFormatKHR surface_format;
		vk::PresentModeKHR present_mode;
		uint32_t image_count;
		vk::Extent2D desired_extent;

		vk::SwapchainKHR swapchain;
		vk::Format format;
		vk::Extent2D extent;
		std::vector<vk::Image> images;
		std::vector<vk::ImageView> image_views;

		static vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &available_formats);
		static vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR> &available_present_modes);
		static vk::Extent2D ChooseExtent(const vk::SurfaceCapabilitiesKHR &capabilities, vk::Extent2D extent);

		void CreateSwapchain();
		void CreateImageViews();

		void Cleanup();

	public:
		Swapchain(Engine *engine, vk::SurfaceKHR surface, vk::Extent2D desired_extent/*,
				  std::optional<vk::SurfaceFormatKHR> force_surface_format = std::nullopt,
				  std::optional<vk::PresentModeKHR> force_present_mode = std::nullopt,
				  std::optional<uint32_t> force_image_count = std::nullopt*/);
		virtual ~Swapchain();

		void Resize(vk::Extent2D extent);
		void Recreate();

		vk::SwapchainKHR GetSwapchain()								{ return swapchain; }

		vk::Extent2D GetExtent() const override;     			//	{ return extent; } // TODO: should be desired_extent?
		vk::Format GetFormat() const override; 					//	{ return format; }
		std::vector<vk::ImageView> GetImageViews() const override;// 	{ return image_views; }
};

}

#endif //LAVOS_SWAPCHAIN_H
