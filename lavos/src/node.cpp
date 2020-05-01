
#include <stdexcept>
#include <functional>
#include <iostream>

#include "lavos/node.h"
#include "lavos/component/transform_component.h"


using namespace lavos;

Node::Node()
{
}

Node::~Node()
{
	for(auto component : components)
		delete component;

	for(auto child : children)
		delete child;
}

void Node::AddComponent(lavos::Component *component)
{
	if(component->GetNode() != nullptr)
		throw std::runtime_error("component is already added to a node.");

	for(const auto &c: components)
	{
		if(c == component)
			return;
	}

	components.push_back(component);

	component->node = this;

	auto transform_component = dynamic_cast<TransformComp *>(component);
	if(transform_component != nullptr)
		this->transform_component = transform_component;
}

void Node::RemoveComponent(Component *component)
{
	if(transform_component == component)
		transform_component = nullptr;

	for(auto it=components.begin(); it!=components.end(); it++)
	{
		if(*it == component)
		{
			components.erase(it);
			component->node = nullptr;
			return;
		}
	}
}

void Node::AddChild(Node *node)
{
	if(node->parent != nullptr && !node->is_root)
		throw std::runtime_error("node is already child of another node or is root node.");

	node->parent = this;
	children.push_back(node);
}

void Node::RemoveChild(Node *node)
{
	if(node->parent != this)
		throw std::runtime_error("node is not child of this node");

	for(auto it=children.begin(); it!=children.end(); it++)
	{
		if(*it == node)
		{
			children.erase(it);
			node->parent = nullptr;
			return;
		}
	}
}

void Node::TraversePreOrder(std::function<void(Node *)> func)
{
	func(this);

	for(auto &child : children)
		child->TraversePreOrder(func);
}

void Node::TraversePostOrder(std::function<void(Node *)> func)
{
	for(auto &child : children)
		child->TraversePreOrder(func);

	func(this);
}
