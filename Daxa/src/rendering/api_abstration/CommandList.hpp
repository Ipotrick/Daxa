#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <variant>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

#include "Image.hpp"
#include "Buffer.hpp"
#include "SwapchainImage.hpp"

namespace daxa {
	namespace gpu {

		struct RenderAttachmentInfo {
			ImageHandle image;
			VkResolveModeFlagBits resolveMode = VK_RESOLVE_MODE_NONE;
			VkAttachmentLoadOp loadOp;
			VkAttachmentStoreOp storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
			VkClearValue clearValue = VkClearValue{ .color = VkClearColorValue{.uint32 = 0x808080FF } };
		};

		struct BeginRenderingInfo {
			RenderAttachmentInfo* colorAttachments = nullptr;
			u32 colorAttachmentCount = 0;
			RenderAttachmentInfo* depthAttachment = nullptr;
			RenderAttachmentInfo* stencilAttachment = nullptr;
			VkRect2D* renderArea = nullptr;
		};

		class CommandList {
		public:
			~CommandList();

			CommandList(CommandList&& rhs) noexcept;
			CommandList& operator=(CommandList&& rhs) noexcept;

			CommandList(CommandList const& rhs) = delete;
			CommandList& operator=(CommandList const& rhs) = delete;

			void begin();
			void end();

			void beginRendering(BeginRenderingInfo ri);
			void endRendering();

			vk::CommandBuffer getVkCommandBuffer() { return cmd; }
		private:
			friend class Device;

			CommandList();
			void reset();

			void (*vkCmdBeginRenderingKHR)(VkCommandBuffer, const VkRenderingInfoKHR*);
			void (*vkCmdEndRenderingKHR)(VkCommandBuffer);
			u32	operationsInProgress = 0;
			bool bIsNotMovedOutOf = true;
			bool empty = true;

			std::vector<VkRenderingAttachmentInfoKHR> renderAttachmentBuffer;

			std::vector<ImageHandle> usedImages;
			std::vector<BufferHandle> usedBuffers;
			vk::Device device;
			vk::CommandBuffer cmd;
			vk::CommandPool cmdPool;
		};

	}
}