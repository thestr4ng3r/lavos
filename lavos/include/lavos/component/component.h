
#ifndef VULKAN_COMPONENT_H
#define VULKAN_COMPONENT_H

namespace lavos
{

class Node;

class Component
{
	friend class Node;

	private:
		Node *node = nullptr;

	public:
		virtual ~Component() {}

		Node *GetNode() const			{ return node; }

		virtual void Update(float delta_time) {}
};

}


#endif //VULKAN_COMPONENT_H
