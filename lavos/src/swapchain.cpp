
#include "lavos/swapchain.h"

using namespace lavos;

Swapchain::Swapchain(Engine *engine, vk::SurfaceKHR surface, vk::Extent2D desired_extent /*,
					 std::optional<vk::SurfaceFormatKHR> force_surface_format,
					 std::optional<vk::PresentModeKHR> force_present_mode,
					 std::optional<uint32_t> force_image_count*/)
	: engine(engine)
{
	this->surface = surface;
	this->desired_extent = desired_extent;

	CreateSwapchain();
	CreateImageViews();
}

Swapchain::~Swapchain()
{
	Cleanup();
}

vk::SurfaceFormatKHR Swapchain::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &available_formats)
{
	const auto preferred_format = vk::Format::eB8G8R8A8Unorm;
	const auto preferred_color_space = vk::ColorSpaceKHR::eSrgbNonlinear;

	if(available_formats.size() == 1 && available_formats[0].format == vk::Format::eUndefined)
	{
		vk::SurfaceFormatKHR r;
		r.format = preferred_format;
		r.colorSpace = preferred_color_space;
		return r;
	}


	for(const auto &format : available_formats)
	{
		if(format.format == preferred_format && format.colorSpace == preferred_color_space)
			return format;
	}

	return available_formats[0];
}

vk::PresentModeKHR Swapchain::ChoosePresentMode(const std::vector<vk::PresentModeKHR> &available_present_modes)
{
	vk::PresentModeKHR best_mode = vk::PresentModeKHR::eFifo;

	for(const auto &present_mode : available_present_modes)
	{
		if(present_mode == vk::PresentModeKHR::eMailbox)
			return present_mode;

		if(present_mode == vk::PresentModeKHR::eImmediate)
			best_mode = present_mode;
	}

	return best_mode;
}

vk::Extent2D Swapchain::ChooseExtent(const vk::SurfaceCapabilitiesKHR &capabilities, vk::Extent2D extent)
{
	if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;

	extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
	extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));
	return extent;
}

void Swapchain::CreateSwapchain()
{
	auto surface_capabilities = engine->GetVkPhysicalDevice().getSurfaceCapabilitiesKHR(surface);
	auto surface_formats = engine->GetVkPhysicalDevice().getSurfaceFormatsKHR(surface);
	auto surface_present_modes = engine->GetVkPhysicalDevice().getSurfacePresentModesKHR(surface);

	surface_format = /*force_surface_format.has_value()
					 ? force_surface_format.value()
					 :*/ ChooseSurfaceFormat(surface_formats);

	present_mode = /*force_present_mode.has_value()
				   ? force_present_mode.value()
				   :*/ ChoosePresentMode(surface_present_modes);

	/*if(force_image_count)
		image_count = force_image_count.value();
	else
	{*/
	image_count = surface_capabilities.minImageCount + 1;
	if(surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount)
		image_count = surface_capabilities.maxImageCount;
	//}

	extent = ChooseExtent(surface_capabilities, desired_extent);

	auto create_info = vk::SwapchainCreateInfoKHR()
			.setSurface(surface)
			.setMinImageCount(image_count)
			.setImageFormat(surface_format.format)
			.setImageColorSpace(surface_format.colorSpace)
			.setImageExtent(extent)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	auto queue_family_indices = engine->GetQueueFamilyIndices();
	uint32_t queue_family_indices_array[] = {static_cast<uint32_t>(queue_family_indices.graphics_family),
											 static_cast<uint32_t>(queue_family_indices.present_family)};

	if(queue_family_indices.graphics_family != queue_family_indices.present_family)
	{
		create_info.setImageSharingMode(vk::SharingMode::eConcurrent)
				.setQueueFamilyIndexCount(2)
				.setPQueueFamilyIndices(queue_family_indices_array);
	}
	else
	{
		create_info.setImageSharingMode(vk::SharingMode::eExclusive);
	}

	create_info.setPreTransform(surface_capabilities.currentTransform)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setPresentMode(present_mode)
			.setClipped(VK_TRUE);
	// TODO .setOldSwapchain(vk::SwapchainKHR(nullptr));

	swapchain = engine->GetVkDevice().createSwapchainKHR(create_info);
	images = engine->GetVkDevice().getSwapchainImagesKHR(swapchain);

	format = surface_format.format;
}

void Swapchain::CreateImageViews()
{
	image_views.resize(images.size());

	for(size_t i=0; i<images.size(); i++)
	{
		image_views[i] = engine->GetVkDevice().createImageView(
				vk::ImageViewCreateInfo()
						.setImage(images[i])
						.setViewType(vk::ImageViewType::e2D)
						.setFormat(format)
						.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
	}
}

void Swapchain::Cleanup()
{
	auto device = engine->GetVkDevice();

	for(const auto &image_view : image_views)
		device.destroyImageView(image_view);

	device.destroySwapchainKHR(swapchain);
}

void Swapchain::Resize(vk::Extent2D extent)
{
	this->desired_extent = extent;
	Recreate();
}

void Swapchain::Recreate()
{
	engine->GetVkDevice().waitIdle(); // TODO: should we really do this?

	Cleanup();
	CreateSwapchain();
	CreateImageViews();

	SignalChangedCallbacks();
}

vk::Extent2D Swapchain::GetExtent() const
{
	return extent; // TODO: should be desired_extent?
}

vk::Format Swapchain::GetFormat() const
{
	return format;
}

std::vector<vk::ImageView> Swapchain::GetImageViews() const
{
	return image_views;
}
