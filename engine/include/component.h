
#ifndef VULKAN_COMPONENT_H
#define VULKAN_COMPONENT_H

namespace engine
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
};

}


#endif //VULKAN_COMPONENT_H
