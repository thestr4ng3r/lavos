
#include "lavos/buffer.h"
#include "lavos/engine.h"

lavos::Buffer::~Buffer()
{
	UnMap();
	engine->DestroyBuffer(buffer, allocation);
}

void *lavos::Buffer::Map()
{
	if(!map)
		map = engine->MapMemory(allocation);

	return map;
}

void lavos::Buffer::UnMap()
{
	if(!map)
		return;
	engine->UnmapMemory(allocation);
	map = nullptr;
}
