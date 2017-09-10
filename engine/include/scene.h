
#ifndef VULKAN_SCENE_H
#define VULKAN_SCENE_H

#include "node.h"

namespace engine
{

class Scene
{
	private:
		Node root_node;

	public:
		Scene();
		~Scene();

		Node *GetRootNode()		{ return &root_node; }
};


}

#endif //VULKAN_SCENE_H
