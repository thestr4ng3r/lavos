
#ifndef LAVOS_RENDER_TARGET_H
#define LAVOS_RENDER_TARGET_H

#include <vector>

#include "engine.h"

namespace lavos
{

class RenderTarget
{
	public:
		class ChangedCallback
		{
				friend class RenderTarget;

			public:
				virtual ~ChangedCallback() {}

			protected:
				virtual void RenderTargetChanged(RenderTarget *render_target) =0;
		};

		virtual vk::Extent2D GetExtent() const =0;
		virtual vk::Format GetFormat() const =0;
		virtual std::vector<vk::ImageView> GetImageViews() const =0;

	private:
		std::vector<ChangedCallback *> changed_callbacks;

	public:
		virtual ~RenderTarget() {}

		void AddChangedCallback(ChangedCallback *callback);
		void RemoveChangedCallback(ChangedCallback *callback);

	protected:
		void SignalChangedCallbacks();
};

}

#endif //LAVOS_RENDER_TARGET_H
