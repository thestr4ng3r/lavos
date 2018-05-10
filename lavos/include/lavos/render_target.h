
#ifndef LAVOS_RENDER_TARGET_H
#define LAVOS_RENDER_TARGET_H

#include <map>

#include "engine.h"

namespace lavos
{

/**
 * A RenderTarget is something that can be rendered into.
 */
class RenderTarget
{
	public:
		class ChangedCallback
		{
				friend class RenderTarget;

			public:
				virtual ~ChangedCallback() = default;

			protected:
				/**
				 * Called e.g. when the {@link RenderTarget}'s extent changes.
				 * @param render_target
				 */
				virtual void RenderTargetChanged(RenderTarget *render_target) =0;
		};

		/**
		 * Defines in which order {@link RenderTarget::ChangedCallback}s are invoked.
		 */
		enum class ChangedCallbackOrder {
			AssociatedRenderTarget = 0,
			Renderer
		};

	private:
		std::multimap<ChangedCallbackOrder, ChangedCallback *> changed_callbacks;

	public:
		virtual ~RenderTarget() = default;

		void AddChangedCallback(ChangedCallbackOrder order, ChangedCallback *callback);
		void RemoveChangedCallback(ChangedCallback *callback);

		virtual vk::Extent2D GetExtent() const =0;

	protected:
		void SignalChangedCallbacks();
};


/**
 * A {@link RenderTarget} to render color into, e.g. a {@link Swapchain}.
 */
class ColorRenderTarget : public RenderTarget
{
	public:
		~ColorRenderTarget() override = default;
		virtual vk::Format GetFormat() const =0;
		virtual std::vector<vk::ImageView> GetImageViews() const =0;
};


/**
 * A {@link RenderTarget} to render depth into.
 */
class DepthRenderTarget : public RenderTarget
{
	public:
		~DepthRenderTarget() override = default;
		virtual vk::Format GetFormat() const =0;
		virtual vk::ImageView GetImageView() const =0;
};


/**
 * A {@link DepthRenderTarget} that is associated with a {@link ColorRenderTarget}
 * and manages the creation of its resources automatically.
 */
class ManagedDepthRenderTarget : public DepthRenderTarget, RenderTarget::ChangedCallback
{
	private:
		Engine * const engine;
		ColorRenderTarget * const color_render_target;

		vk::Format format;
		lavos::Image image;
		vk::ImageView image_view;

		void CreateResources();
		void CleanupResources();

	protected:
		void RenderTargetChanged(RenderTarget *render_target);

	public:
		ManagedDepthRenderTarget(Engine *engine, ColorRenderTarget *color_render_target);
		~ManagedDepthRenderTarget() override;

		virtual vk::Extent2D GetExtent() const	{ return color_render_target->GetExtent(); }
		vk::Format GetFormat() const			{ return format; }
		vk::ImageView GetImageView() const  	{ return image_view; }
};


}

#endif //LAVOS_RENDER_TARGET_H
