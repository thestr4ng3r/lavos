
#ifndef VULKAN_NODE_H
#define VULKAN_NODE_H

#include <vector>
#include <functional>

#include "component.h"
#include "transform_component.h"

namespace engine
{

class Node
{
	friend class Scene;

	private:
		bool is_root = false;
		Node *parent = nullptr;

		std::vector<Component *> components;
		std::vector<Node *> children;

		TransformComponent *transform_component = nullptr;

	public:
		Node();
		~Node();

		template<class T>
		T *GetComponent() const;

		template<class T>
		std::vector<T *> GetComponents() const;

		TransformComponent *GetTransformComponent() const	{ return transform_component; }


		const std::vector<Node *> &GetChildren() const		{ return children; }


		void AddComponent(Component *component);
		void RemoveComponent(Component *component);

		void AddChild(Node *node);
		void RemoveChild(Node *node);

		void TraversePreOrder(std::function<void (Node *)> func);
		void TraversePostOrder(std::function<void (Node *)> func);
};


template<class T>
inline T *Node::GetComponent() const
{
	for(auto &component: components)
	{
		auto found_component = dynamic_cast<T *>(component);
		if(found_component != nullptr)
			return found_component;
	}

	return nullptr;
}

template<class T>
inline std::vector<T *> Node::GetComponents() const
{
	std::vector<T *> r;

	for(auto &component: components)
	{
		auto found_component = dynamic_cast<T *>(component);
		if(found_component != nullptr)
			r.push_back(found_component);
	}

	return r;
}

}


#endif //VULKAN_NODE_H
