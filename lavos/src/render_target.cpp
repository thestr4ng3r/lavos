
#include "lavos/render_target.h"

using namespace lavos;

void RenderTarget::AddChangedCallback(RenderTarget::ChangedCallback *callback)
{
	RemoveChangedCallback(callback);
	changed_callbacks.push_back(callback);
}

void RenderTarget::RemoveChangedCallback(RenderTarget::ChangedCallback *callback)
{
	for(auto i=changed_callbacks.begin(); i!=changed_callbacks.end(); i++)
	{
		if(*i == callback)
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
		callback->RenderTargetChanged(this);
	}
}
