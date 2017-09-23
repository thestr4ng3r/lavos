
#ifdef ENGINE_IMPLEMENT_STB_IMAGE
#  define STB_IMAGE_IMPLEMENTATION
#  include "../../thirdparty/stb_image.h"
#  undef STB_IMAGE_IMPLEMENTATION
#endif

#ifdef ENGINE_IMPLEMENT_TINYGLTF
#  define TINYGLTF_IMPLEMENTATION
#  include "../../thirdparty/tiny_gltf.h"
#  undef TINYGLTF_IMPLEMENTATION
#endif

#ifdef ENGINE_IMPLEMENT_VMA
// TODO: revise this workaround for android:
#  include <malloc.h>
#  define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment)   (memalign((alignment), (size)))

#  define VMA_IMPLEMENTATION
#  include "vk_mem_alloc.h"
#  undef VMA_IMPLEMENTATION
#endif