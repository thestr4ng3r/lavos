
#include <stdexcept>
#include "node.h"

using namespace engine;

Node::Node()
	: is_root(false), parent(nullptr)
{
}

Node::~Node()
{
	for(auto component : components)
		delete component;

	for(auto child : children)
		delete child;
}

void Node::AddComponent(engine::Component *component)
{
	for(const auto &c: components)
	{
		if(c == component)
			return;
	}

	components.push_back(component);
}

void Node::RemoveComponent(Component *component)
{
	for(auto it=components.begin(); it!=components.end(); it++)
	{
		if(*it == component)
		{
			components.erase(it);
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
