#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <variant>
#include <span>

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
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			VkAttachmentStoreOp storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
			VkClearValue clearValue = VkClearValue{ .color = VkClearColorValue{.float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } };
		};

		struct BeginRenderingInfo {
			std::span<RenderAttachmentInfo> colorAttachments;
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

			void changeImageLayout(ImageHandle image, VkImageLayout newLayout);

			void setViewport(VkViewport const& viewport);

			void setScissor(VkRect2D const& scissor);

			void draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);

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