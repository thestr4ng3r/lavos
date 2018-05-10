
#include "lavos/render_target.h"

using namespace lavos;

void RenderTarget::AddChangedCallback(ChangedCallbackOrder order, ChangedCallback *callback)
{
	RemoveChangedCallback(callback);
	changed_callbacks.insert(std::pair<ChangedCallbackOrder, ChangedCallback *>(order, callback));
}

void RenderTarget::RemoveChangedCallback(ColorRenderTarget::ChangedCallback *callback)
{
	for(auto i=changed_callbacks.begin(); i!=changed_callbacks.end(); i++)
	{
		if(i->second == callback)
		{
			changed_callbacks.erase(i);
			return;
		}
	}
}

void RenderTarget::SignalChangedCallbacks()
{
	for(auto callback : changed_callbacks)
	{
		callback.second->RenderTargetChanged(this);
	}
}


// ------------------------------------------


ManagedDepthRenderTarget::ManagedDepthRenderTarget(Engine *engine, ColorRenderTarget *color_render_target)
		: engine(engine),
		  color_render_target(color_render_target)
{
	CreateResources();
	color_render_target->AddChangedCallback(RenderTarget::ChangedCallbackOrder::AssociatedRenderTarget, this);
}

ManagedDepthRenderTarget::~ManagedDepthRenderTarget()
{
	CleanupResources();
}

void ManagedDepthRenderTarget::CreateResources()
{
	auto extent = color_render_target->GetExtent();

	format = engine->FindDepthFormat();

	image = engine->Create2DImage(extent.width, extent.height, format,
								  vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
								  VMA_MEMORY_USAGE_GPU_ONLY);

	image_view = engine->GetVkDevice().createImageView(vk::ImageViewCreateInfo()
															   .setImage(image.image)
															   .setViewType(vk::ImageViewType::e2D)
															   .setFormat(format)
															   .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1)));

	engine->TransitionImageLayout(image.image, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

void ManagedDepthRenderTarget::CleanupResources()
{
	engine->GetVkDevice().destroyImageView(image_view);
	engine->DestroyImage(image);
}

void ManagedDepthRenderTarget::RenderTargetChanged(RenderTarget *render_target)
{
	CleanupResources();
	CreateResources();
}