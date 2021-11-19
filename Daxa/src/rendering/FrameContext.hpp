#pragma once

#include <unordered_map>
#include <string_view>

#include "Vulkan.hpp"

#include "Buffer.hpp"

namespace daxa {

	struct FrameContext {
		FrameContext(GPUContext& gpu) 
			: fence{ gpu.device.createFenceUnique({}) }
			, commandBufferPool{ gpu.device, {} }
			, semaPool{
				[]() { return VulkanGlobals::device.createSemaphore({}); },
				[](vk::Semaphore sem) { VulkanGlobals::device.destroySemaphore(sem,nullptr); },
				[](vk::Semaphore sem) { /* dont need to reset a semaphore */ }
			}
		{

		}

		void reset() {
			commandBufferPool.reset();
			semaPool.reset();
			temporaryFramebuffers.clear();
		}

		void waitOnCompletion(GPUContext& gpu) {
			gpu.device.waitForFences(*fence, VK_TRUE, 1000000000);
			gpu.device.resetFences(*fence);
		}

		// persistant:
		vk::UniqueFence fence;
		std::unordered_map<std::string_view, vk::DescriptorSet> descriptorSets;
		std::unordered_map<std::string_view, Buffer> buffers;

		// temporaries:
		vkh::CommandBufferPool commandBufferPool;
		vkh::Pool<vk::Semaphore> semaPool;
		std::vector<vk::UniqueFramebuffer> temporaryFramebuffers;
		std::vector<vk::UniqueRenderPass> temporaryRenderPasses;
	};

}