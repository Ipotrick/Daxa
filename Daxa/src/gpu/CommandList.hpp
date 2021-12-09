#pragma once

#include "../DaxaCore.hpp"

#include <variant>
#include <span>
#include <vector>
#include <optional>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "Image.hpp"
#include "Buffer.hpp"
#include "SwapchainImage.hpp"
#include "Pipeline.hpp"
#include "BindingSet.hpp"
#include "StagingBufferPool.hpp"

namespace daxa {
	namespace gpu {

		/**
		 * use this for all sync inside a queue:
		*/
		struct MemoryBarrier {
			VkPipelineStageFlags2KHR awaitedStages = 0;
			VkAccessFlags2KHR awaitedAccess = 0;
			VkPipelineStageFlags2KHR waitingStages = 0;
			VkAccessFlags2KHR waitingAccess = 0;
		};

		/**
		 * no real use case apparently:
		*/
		struct BufferBarrier {
			VkPipelineStageFlags2KHR awaitedStages = 0;
			VkAccessFlags2KHR awaitedAccess = 0;
			VkPipelineStageFlags2KHR waitingStages = 0;
			VkAccessFlags2KHR waitingAccess = 0;
			BufferHandle buffer = {};
			size_t offset = 0;
			std::optional<u32> size = {};
		};

		/**
		 * used for layout changes
		*/
		struct ImageBarrier {
			VkPipelineStageFlags2KHR awaitedStages = 0;
			VkAccessFlags2KHR awaitedAccess = 0;
			VkPipelineStageFlags2KHR waitingStages = 0;
			VkAccessFlags2KHR waitingAccess = 0;
			ImageHandle image = {};
			VkImageLayout layoutBefore;
			VkImageLayout layoutAfter;
			std::optional<VkImageSubresourceRange> subRange = {};
		};

		struct RenderAttachmentInfo {
			ImageHandle image;
			VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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

			template<typename T, size_t N>
			void uploadToBuffer(std::array<T, N> const& arraySrc, BufferHandle dst, size_t dstOffset = 0) {
				uploadToBuffer(arraySrc.data(), sizeof(T) * N, std::move(dst), dstOffset);
			}
			
			template<typename T>
				void uploadToBuffer(std::vector<T> const& vecSrc, BufferHandle dst, size_t dstOffset = 0) {
				uploadToBuffer(vecSrc.data(), sizeof(T) * vecSrc.size(), std::move(dst), dstOffset);
			}

			void uploadToBuffer(void const* src, size_t size, BufferHandle dst, size_t dstOffset = 0);

			void copyBufferToBuffer(BufferHandle src, BufferHandle dst, VkBufferCopy region) {
				copyBufferToBufferMulti(src, dst, { &region, 1 });
			}

			void copyBufferToBufferMulti(BufferHandle src, BufferHandle dst, std::span<VkBufferCopy> copyRegions);

			void copyBufferToImage(BufferHandle src, size_t srcOffset, size_t size, ImageHandle dst, std::optional<VkImageSubresourceLayers> dstSubRessource = {});

			void uploadToImage(void const* src, size_t size, ImageHandle dst, std::optional<VkImageSubresourceLayers> dstSubRessource = {});

			void uploadToImageSynced(void const* src, size_t size, ImageHandle dst, VkImageLayout dstLayout, std::optional<VkImageSubresourceLayers> dstSubRessource = {});

			// Binding set management:

			void updateSetImages(BindingSetHandle& set, u32 binding, std::span<std::pair<ImageHandle, VkImageLayout>> images, u32 descriptorArrayOffset = 0);
			void updateSetImage(BindingSetHandle& set, u32 binding, ImageHandle image, VkImageLayout layout);

			void updateSetBuffers(BindingSetHandle& set, u32 binding, std::span<BufferHandle> buffers, u32 descriptorArrayOffset = 0);
			void updateSetBuffer(BindingSetHandle& set, u32 binding, BufferHandle buffer);

			void updateSetSamplers(BindingSetHandle& set, u32 binding, std::span<SamplerHandle> samplers, u32 descriptorArrayOffset = 0);
			void updateSetSampler(BindingSetHandle& set, u32 binding, SamplerHandle sampler);

			void bindSet(u32 setBinding, BindingSetHandle& set);

			// Rendering:

			void bindVertexBuffer(u32 binding, BufferHandle buffer, size_t bufferOffset = 0);

			// TODO REFACTOR, THIS LEADS TO BUGS (EXAMPLE: GIVE VECTOR AS CONSTANT PARAMETER)
			template<typename T>
			void pushConstant(VkShaderStageFlagBits shaderStage, T& constant, size_t offset = 0) {
				vkCmdPushConstants(cmd, boundPipeline.value().layout, shaderStage, offset, sizeof(T), &constant);
			}

			void beginRendering(BeginRenderingInfo ri);

			void endRendering();

			void bindPipeline(GraphicsPipelineHandle graphicsPipeline);

			void setViewport(VkViewport const& viewport);

			void setScissor(VkRect2D const& scissor);

			void draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);

			// sync:

			void insertBarriers(std::span<MemoryBarrier> memBarriers, std::span<BufferBarrier> bufBarriers, std::span<ImageBarrier> imgBarriers);

			// Accessors:

			VkCommandBuffer getVkCommandBuffer() { return cmd; }
		private:
			friend class Device;
			friend class Queue;

			CommandList();

			void reset();

			// binding set management:
			std::vector<VkDescriptorBufferInfo> bufferInfoBuffer;
			std::vector<VkDescriptorImageInfo> imageInfoBuffer;
			// begin rendering temporaries:
			std::vector<VkRenderingAttachmentInfoKHR> renderAttachmentBuffer;

			struct BoundPipeline {
				VkPipelineBindPoint bindPoint;
				VkPipelineLayout layout;
			};

			std::optional<BoundPipeline> boundPipeline;

			void (*vkCmdBeginRenderingKHR)(VkCommandBuffer, const VkRenderingInfoKHR*);
			void (*vkCmdEndRenderingKHR)(VkCommandBuffer);
			void (*vkCmdPipelineBarrier2KHR)(VkCommandBuffer, VkDependencyInfoKHR const*);
			u32	operationsInProgress = 0;
			bool empty = true;

			std::vector<BindingSetHandle> usedSets;
			std::vector<ImageHandle> usedImages;
			std::vector<BufferHandle> usedBuffers;
			std::vector<GraphicsPipelineHandle> usedGraphicsPipelines;
			VkDevice device;
			VkCommandBuffer cmd;
			VkCommandPool cmdPool;

			std::weak_ptr<StagingBufferPool> stagingBufferPool = {};
			std::vector<StagingBuffer> usedStagingBuffers;
		};
	}
}