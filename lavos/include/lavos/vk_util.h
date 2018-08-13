
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

}}

#endif //LAVOS_VK_UTIL_H
