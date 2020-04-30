
#ifndef LAVOS_VK_UTIL_H
#define LAVOS_VK_UTIL_H

#include <vulkan/vulkan.hpp>
#include <vector>

namespace lavos { namespace vk_util {

/**
 * Enclosing vk::PipelineColorBlendStateCreateInfo,
 * but owning the memory for the attachments instead of just referencing it.
 * (for RAII)
 */
class PipelineColorBlendStateCreateInfo
{
	private:
		std::vector<vk::PipelineColorBlendAttachmentState> attachments;
		vk::PipelineColorBlendStateCreateInfo info;

	public:
		explicit PipelineColorBlendStateCreateInfo(
					vk::PipelineColorBlendStateCreateFlags flags = vk::PipelineColorBlendStateCreateFlags(),
					vk::Bool32 logicOpEnable = 0,
					vk::LogicOp logicOp = vk::LogicOp::eClear,
					const std::vector<vk::PipelineColorBlendAttachmentState> &attachments = {},
					const std::array<float, 4> &blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }) :
				attachments(attachments),
				info(flags, logicOpEnable, logicOp, 0, nullptr, blendConstants) {}

		const vk::PipelineColorBlendStateCreateInfo &Get()
		{
			info.setAttachmentCount(static_cast<uint32_t>(attachments.size()));
			info.setPAttachments(attachments.data());
			return info;
		}

		PipelineColorBlendStateCreateInfo &SetPNext(const void* pNext)
		{
			info.setPNext(pNext);
			return *this;
		}

		PipelineColorBlendStateCreateInfo &SetFlags(vk::PipelineColorBlendStateCreateFlags flags)
		{
			info.setFlags(flags);
			return *this;
		}

		PipelineColorBlendStateCreateInfo &SetLogicOpEnable(vk::Bool32 logicOpEnable)
		{
			info.setLogicOpEnable(logicOpEnable);
			return *this;
		}

		PipelineColorBlendStateCreateInfo &SetLogicOp(vk::LogicOp logicOp)
		{
			info.setLogicOp(logicOp);
			return *this;
		}

		PipelineColorBlendStateCreateInfo &SetAttachments(const std::vector<vk::PipelineColorBlendAttachmentState> &attachments_)
		{
			attachments = attachments_;
			return *this;
		}

		PipelineColorBlendStateCreateInfo &SetBlendConstants(std::array<float,4> blendConstants)
		{
			info.setBlendConstants(blendConstants);
			return *this;
		}
};

template<typename T>
vk::ObjectType GetObjectType();

template<> inline vk::ObjectType GetObjectType<vk::Instance>()					{ return vk::ObjectType::eInstance; }
template<> inline vk::ObjectType GetObjectType<vk::PhysicalDevice>()			{ return vk::ObjectType::ePhysicalDevice; }
template<> inline vk::ObjectType GetObjectType<vk::Device>()					{ return vk::ObjectType::eDevice; }
template<> inline vk::ObjectType GetObjectType<vk::Queue>()						{ return vk::ObjectType::eQueue; }
template<> inline vk::ObjectType GetObjectType<vk::Semaphore>()					{ return vk::ObjectType::eSemaphore; }
template<> inline vk::ObjectType GetObjectType<vk::CommandBuffer>()				{ return vk::ObjectType::eCommandBuffer; }
template<> inline vk::ObjectType GetObjectType<vk::Fence>()						{ return vk::ObjectType::eFence; }
template<> inline vk::ObjectType GetObjectType<vk::DeviceMemory>()				{ return vk::ObjectType::eDeviceMemory; }
template<> inline vk::ObjectType GetObjectType<vk::Buffer>()					{ return vk::ObjectType::eBuffer; }
template<> inline vk::ObjectType GetObjectType<vk::Image>()						{ return vk::ObjectType::eImage; }
template<> inline vk::ObjectType GetObjectType<vk::Event>()						{ return vk::ObjectType::eEvent; }
template<> inline vk::ObjectType GetObjectType<vk::QueryPool>()					{ return vk::ObjectType::eQueryPool; }
template<> inline vk::ObjectType GetObjectType<vk::BufferView>()				{ return vk::ObjectType::eBufferView; }
template<> inline vk::ObjectType GetObjectType<vk::ImageView>()					{ return vk::ObjectType::eImageView; }
template<> inline vk::ObjectType GetObjectType<vk::ShaderModule>()				{ return vk::ObjectType::eShaderModule; }
template<> inline vk::ObjectType GetObjectType<vk::PipelineCache>()				{ return vk::ObjectType::ePipelineCache; }
template<> inline vk::ObjectType GetObjectType<vk::PipelineLayout>()			{ return vk::ObjectType::ePipelineLayout; }
template<> inline vk::ObjectType GetObjectType<vk::RenderPass>()				{ return vk::ObjectType::eRenderPass; }
template<> inline vk::ObjectType GetObjectType<vk::Pipeline>()					{ return vk::ObjectType::ePipeline; }
template<> inline vk::ObjectType GetObjectType<vk::DescriptorSetLayout>()		{ return vk::ObjectType::eDescriptorSetLayout; }
template<> inline vk::ObjectType GetObjectType<vk::Sampler>()					{ return vk::ObjectType::eSampler; }
template<> inline vk::ObjectType GetObjectType<vk::DescriptorPool>()			{ return vk::ObjectType::eDescriptorPool; }
template<> inline vk::ObjectType GetObjectType<vk::DescriptorSet>()				{ return vk::ObjectType::eDescriptorSet; }
template<> inline vk::ObjectType GetObjectType<vk::Framebuffer>()				{ return vk::ObjectType::eFramebuffer; }
template<> inline vk::ObjectType GetObjectType<vk::CommandPool>()				{ return vk::ObjectType::eCommandPool; }
template<> inline vk::ObjectType GetObjectType<vk::SamplerYcbcrConversion>()	{ return vk::ObjectType::eSamplerYcbcrConversion; }
template<> inline vk::ObjectType GetObjectType<vk::DescriptorUpdateTemplate>()	{ return vk::ObjectType::eDescriptorUpdateTemplate; }
template<> inline vk::ObjectType GetObjectType<vk::SurfaceKHR>()				{ return vk::ObjectType::eSurfaceKHR; }
template<> inline vk::ObjectType GetObjectType<vk::SwapchainKHR>()				{ return vk::ObjectType::eSwapchainKHR; }
template<> inline vk::ObjectType GetObjectType<vk::DisplayKHR>()				{ return vk::ObjectType::eDisplayKHR; }
template<> inline vk::ObjectType GetObjectType<vk::DisplayModeKHR>()			{ return vk::ObjectType::eDisplayModeKHR; }
template<> inline vk::ObjectType GetObjectType<vk::DebugReportCallbackEXT>()	{ return vk::ObjectType::eDebugReportCallbackEXT; }
//template<> inline vk::ObjectType GetObjectType<vk::ObjectTableNVX>()			{ return vk::ObjectType::eObjectTableNVX; }
//template<> inline vk::ObjectType GetObjectType<vk::IndirectCommandsLayoutNVX>()	{ return vk::ObjectType::eIndirectCommandsLayoutNVX; }
template<> inline vk::ObjectType GetObjectType<vk::DebugUtilsMessengerEXT>()	{ return vk::ObjectType::eDebugUtilsMessengerEXT; }
template<> inline vk::ObjectType GetObjectType<vk::ValidationCacheEXT>()		{ return vk::ObjectType::eValidationCacheEXT; }


template<typename T>
uint64_t GetHandle(T object);

template<> inline uint64_t GetHandle<vk::Instance>(vk::Instance object)										{ return reinterpret_cast<uint64_t>(static_cast<VkInstance>(object)); }
template<> inline uint64_t GetHandle<vk::PhysicalDevice>(vk::PhysicalDevice object)							{ return reinterpret_cast<uint64_t>(static_cast<VkPhysicalDevice>(object)); }
template<> inline uint64_t GetHandle<vk::Device>(vk::Device object)											{ return reinterpret_cast<uint64_t>(static_cast<VkDevice>(object)); }
template<> inline uint64_t GetHandle<vk::Queue>(vk::Queue object)											{ return reinterpret_cast<uint64_t>(static_cast<VkQueue>(object)); }
template<> inline uint64_t GetHandle<vk::Semaphore>(vk::Semaphore object)									{ return reinterpret_cast<uint64_t>(static_cast<VkSemaphore>(object)); }
template<> inline uint64_t GetHandle<vk::CommandBuffer>(vk::CommandBuffer object)							{ return reinterpret_cast<uint64_t>(static_cast<VkCommandBuffer>(object)); }
template<> inline uint64_t GetHandle<vk::Fence>(vk::Fence object)											{ return reinterpret_cast<uint64_t>(static_cast<VkFence>(object)); }
template<> inline uint64_t GetHandle<vk::DeviceMemory>(vk::DeviceMemory object)								{ return reinterpret_cast<uint64_t>(static_cast<VkDeviceMemory>(object)); }
template<> inline uint64_t GetHandle<vk::Buffer>(vk::Buffer object)											{ return reinterpret_cast<uint64_t>(static_cast<VkBuffer>(object)); }
template<> inline uint64_t GetHandle<vk::Image>(vk::Image object)											{ return reinterpret_cast<uint64_t>(static_cast<VkImage>(object)); }
template<> inline uint64_t GetHandle<vk::Event>(vk::Event object)											{ return reinterpret_cast<uint64_t>(static_cast<VkEvent>(object)); }
template<> inline uint64_t GetHandle<vk::QueryPool>(vk::QueryPool object)									{ return reinterpret_cast<uint64_t>(static_cast<VkQueryPool>(object)); }
template<> inline uint64_t GetHandle<vk::BufferView>(vk::BufferView object)									{ return reinterpret_cast<uint64_t>(static_cast<VkBufferView>(object)); }
template<> inline uint64_t GetHandle<vk::ImageView>(vk::ImageView object)									{ return reinterpret_cast<uint64_t>(static_cast<VkImageView>(object)); }
template<> inline uint64_t GetHandle<vk::ShaderModule>(vk::ShaderModule object)								{ return reinterpret_cast<uint64_t>(static_cast<VkShaderModule>(object)); }
template<> inline uint64_t GetHandle<vk::PipelineCache>(vk::PipelineCache object)							{ return reinterpret_cast<uint64_t>(static_cast<VkPipelineCache>(object)); }
template<> inline uint64_t GetHandle<vk::PipelineLayout>(vk::PipelineLayout object)							{ return reinterpret_cast<uint64_t>(static_cast<VkPipelineLayout>(object)); }
template<> inline uint64_t GetHandle<vk::RenderPass>(vk::RenderPass object)									{ return reinterpret_cast<uint64_t>(static_cast<VkRenderPass>(object)); }
template<> inline uint64_t GetHandle<vk::Pipeline>(vk::Pipeline object)										{ return reinterpret_cast<uint64_t>(static_cast<VkPipeline>(object)); }
template<> inline uint64_t GetHandle<vk::DescriptorSetLayout>(vk::DescriptorSetLayout object)				{ return reinterpret_cast<uint64_t>(static_cast<VkDescriptorSetLayout>(object)); }
template<> inline uint64_t GetHandle<vk::Sampler>(vk::Sampler object)										{ return reinterpret_cast<uint64_t>(static_cast<VkSampler>(object)); }
template<> inline uint64_t GetHandle<vk::DescriptorPool>(vk::DescriptorPool object)							{ return reinterpret_cast<uint64_t>(static_cast<VkDescriptorPool>(object)); }
template<> inline uint64_t GetHandle<vk::DescriptorSet>(vk::DescriptorSet object)							{ return reinterpret_cast<uint64_t>(static_cast<VkDescriptorSet>(object)); }
template<> inline uint64_t GetHandle<vk::Framebuffer>(vk::Framebuffer object)								{ return reinterpret_cast<uint64_t>(static_cast<VkFramebuffer>(object)); }
template<> inline uint64_t GetHandle<vk::CommandPool>(vk::CommandPool object)								{ return reinterpret_cast<uint64_t>(static_cast<VkCommandPool>(object)); }
template<> inline uint64_t GetHandle<vk::SamplerYcbcrConversion>(vk::SamplerYcbcrConversion object)			{ return reinterpret_cast<uint64_t>(static_cast<VkSamplerYcbcrConversion>(object)); }
template<> inline uint64_t GetHandle<vk::DescriptorUpdateTemplate>(vk::DescriptorUpdateTemplate object)		{ return reinterpret_cast<uint64_t>(static_cast<VkDescriptorUpdateTemplate>(object)); }
template<> inline uint64_t GetHandle<vk::SurfaceKHR>(vk::SurfaceKHR object)									{ return reinterpret_cast<uint64_t>(static_cast<VkSurfaceKHR>(object)); }
template<> inline uint64_t GetHandle<vk::SwapchainKHR>(vk::SwapchainKHR object)								{ return reinterpret_cast<uint64_t>(static_cast<VkSwapchainKHR>(object)); }
template<> inline uint64_t GetHandle<vk::DisplayKHR>(vk::DisplayKHR object)									{ return reinterpret_cast<uint64_t>(static_cast<VkDisplayKHR>(object)); }
template<> inline uint64_t GetHandle<vk::DisplayModeKHR>(vk::DisplayModeKHR object)							{ return reinterpret_cast<uint64_t>(static_cast<VkDisplayModeKHR>(object)); }
template<> inline uint64_t GetHandle<vk::DebugReportCallbackEXT>(vk::DebugReportCallbackEXT object)			{ return reinterpret_cast<uint64_t>(static_cast<VkDebugReportCallbackEXT>(object)); }
//template<> inline uint64_t GetHandle<vk::ObjectTableNVX>(vk::ObjectTableNVX object)							{ return reinterpret_cast<uint64_t>(static_cast<VkObjectTableNVX>(object)); }
//template<> inline uint64_t GetHandle<vk::IndirectCommandsLayoutNVX>(vk::IndirectCommandsLayoutNVX object)	{ return reinterpret_cast<uint64_t>(static_cast<VkIndirectCommandsLayoutNVX>(object)); }
template<> inline uint64_t GetHandle<vk::DebugUtilsMessengerEXT>(vk::DebugUtilsMessengerEXT object)			{ return reinterpret_cast<uint64_t>(static_cast<VkDebugUtilsMessengerEXT>(object)); }
template<> inline uint64_t GetHandle<vk::ValidationCacheEXT>(vk::ValidationCacheEXT object)					{ return reinterpret_cast<uint64_t>(static_cast<VkValidationCacheEXT>(object)); }


template<typename T>
inline void SetDebugUtilsObjectName(vk::Device device, T object, const char *name)
{
#ifdef LAVOS_ENABLE_VK_NAMES
	auto info = vk::DebugUtilsObjectNameInfoEXT()
			.setObjectType(GetObjectType<T>())
			.setObjectHandle(GetHandle<T>(object))
			.setPObjectName(name);
	device.setDebugUtilsObjectNameEXT(info);
#else
	(void)device; (void)object; (void)name;
#endif
}


}}

#endif //LAVOS_VK_UTIL_H
