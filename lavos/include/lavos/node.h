
#ifndef LAVOS_NODE_H
#define LAVOS_NODE_H

#include <vector>
#include <functional>

#include "component/component.h"

namespace lavos
{

class TransformComp;

class Node
{
	friend class Scene;

	private:
		bool is_root = false;
		Node *parent = nullptr;

		std::vector<Component *> components;
		std::vector<Node *> children;

		TransformComp *transform_component = nullptr;

	public:
		Node();
		~Node();

		template<class T>
		T *GetComponent() const;

		template<class T>
		std::vector<T *> GetComponents() const;

		template<class T>
		T *GetComponentInChildren() const;

		template<class T>
		std::vector<T *> GetComponentsInChildren() const;

		TransformComp *GetTransformComp() const	{ return transform_component; }


		const std::vector<Node *> &GetChildren() const		{ return children; }

		Node *GetParent() const 							{ return parent; }

		void AddComponent(Component *component);
		void RemoveComponent(Component *component);

		void AddChild(Node *node);
		void RemoveChild(Node *node);

		void TraversePreOrder(std::function<void (Node *)> func);
		void TraversePostOrder(std::function<void (Node *)> func);

		void Update(float delta_time)
		{
			for(auto component : components)
				component->Update(delta_time);

			for(auto child : children)
				child->Update(delta_time);
		}
};


template<class T>
inline T *Node::GetComponent() const
{
	for(auto component: components)
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

	for(auto component: components)
	{
		auto found_component = dynamic_cast<T *>(component);
		if(found_component != nullptr)
			r.push_back(found_component);
	}

	return r;
}

template<class T>
inline T *Node::GetComponentInChildren() const
{
	auto found_component = GetComponent<T>();
	if(found_component != nullptr)
		return found_component;

	for(auto child: children)
	{
		found_component = child->GetComponentInChildren<T>();
		if(found_component != nullptr)
			return found_component;
	}

	return nullptr;
}

template<class T>
inline std::vector<T *> Node::GetComponentsInChildren() const
{
	std::vector<T *> r = GetComponents<T>();

	for(auto child: children)
	{
		auto child_components = child->GetComponentsInChildren<T>();
		r.insert(r.end(), child_components.begin(), child_components.end());
	}

	return r;
}

}


#endif //VULKAN_NODE_H
