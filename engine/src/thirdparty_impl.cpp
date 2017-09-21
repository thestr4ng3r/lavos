
#ifndef __ANDROID__
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "../../thirdparty/tiny_gltf.h"
#endif

#include <stdlib.h>
#include <malloc.h>

#define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment)   (memalign((alignment), (size) ))

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"