#pragma once

#include "../../DaxaCore.hpp"

#include <variant>
#include <span>
#include <vector>
#include <optional>

#include <vulkan/vulkan.h>

#include "../dependencies/vk_mem_alloc.hpp"

#include "Image.hpp"
#include "Buffer.hpp"
#include "SwapchainImage.hpp"
#include "Pipeline.hpp"
#include "BindingSet.hpp"

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

			// Ressource management:

			void changeImageLayout(ImageHandle image, VkImageLayout newLayout);

			void copyBufferToBuffer(BufferHandle src, BufferHandle dst, std::span<VkBufferCopy> copyRegions);

			// Binding set management:

		private:
			std::vector<VkDescriptorBufferInfo> bufferInfoBuffer;
			std::vector<VkDescriptorImageInfo> imageInfoBuffer;
		public:

			void updateSetImages(BindingSetHandle& set, u32 binding, std::span<ImageHandle> images, u32 descriptorArrayOffset = 0);
			void updateSetImage(BindingSetHandle& set, u32 binding, ImageHandle image);

			void updateSetBuffers(BindingSetHandle& set, u32 binding, std::span<BufferHandle> buffers, u32 descriptorArrayOffset = 0);
			void updateSetBuffer(BindingSetHandle& set, u32 binding, BufferHandle buffer);

			void updateSetSamplers() { /* TODO */ }
			void updateSetSampler() { /* TODO */ }

			void bindSet(BindingSetHandle& set);

			// Rendering:

			void beginRendering(BeginRenderingInfo ri);

			void endRendering();

			void bindPipeline(GraphicsPipelineHandle graphicsPipeline);

			void setViewport(VkViewport const& viewport);

			void setScissor(VkRect2D const& scissor);

			void draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);

			// Accessors:

			VkCommandBuffer getVkCommandBuffer() { return cmd; }
		private:
			friend class Device;

			CommandList();

			void reset();

			struct BoundPipeline {
				VkPipelineBindPoint bindPoint;
				VkPipelineLayout layout;
			};

			std::optional<BoundPipeline> boundPipeline;

			void (*vkCmdBeginRenderingKHR)(VkCommandBuffer, const VkRenderingInfoKHR*);
			void (*vkCmdEndRenderingKHR)(VkCommandBuffer);
			u32	operationsInProgress = 0;
			bool empty = true;
			std::vector<VkRenderingAttachmentInfoKHR> renderAttachmentBuffer;
			std::vector<ImageHandle> usedImages;
			std::vector<BufferHandle> usedBuffers;
			std::vector<GraphicsPipelineHandle> usedGraphicsPipelines;
			VkDevice device;
			VkCommandBuffer cmd;
			VkCommandPool cmdPool;
		};
	}
}