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

		class MappedStagingMemory{
		public:
			MappedStagingMemory(void* hostPtr, size_t size, BufferHandle buffer)
				: hostPtr{ hostPtr }
				, size{ size }
				, buffer{ std::move(buffer) }
			{}
			void* const hostPtr;
			size_t const size;
		private:
			friend class CommandList;
			friend class Queue;

			BufferHandle buffer = {};
		};

		struct BufferCopyRegion {
			size_t srcOffset = 0;
			size_t dstOffset = 0;
			size_t size = 0;
		};

		struct HostToBufferCopyInfo {
			void* src = nullptr;
			BufferHandle dst = {};
			size_t dstOffset = 0;
			size_t size = 0;
		};

		struct BufferToBufferMultiCopyInfo {
			BufferHandle src = {};
			BufferHandle dst = {};
			std::span<BufferCopyRegion> regions;
		};

		struct BufferToBufferCopyInfo {
			BufferHandle src = {};
			BufferHandle dst = {};
			BufferCopyRegion region = {};
		};

		struct BufferToImageCopyInfo {
			BufferHandle src = {};
			ImageHandle dst = {};
			size_t srcOffset = 0;
			std::optional<VkImageSubresourceLayers> subRessourceLayers = std::nullopt;
			size_t size = 0;
		};

		struct HostToImageCopyInfo {
			void* src = nullptr;
			ImageHandle dst = {};
			std::optional<VkImageSubresourceLayers> dstImgSubressource = std::nullopt;
			size_t size = 0;
		};

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
			ImageHandle image = {};
			VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			VkResolveModeFlagBits resolveMode = VK_RESOLVE_MODE_NONE;
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			VkClearValue clearValue = VkClearValue{ .color = VkClearColorValue{.float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } };
		};

		struct BeginRenderingInfo {
			std::span<RenderAttachmentInfo> colorAttachments;
			RenderAttachmentInfo* depthAttachment = nullptr;
			RenderAttachmentInfo* stencilAttachment = nullptr;
			VkRect2D* renderArea = nullptr;
		};
		
		class CommandList;

		struct CommandListRecyclingSharedData {
			std::mutex mut = {};
			std::vector<std::shared_ptr<CommandList>> zombies;
		};

		class CommandList {
		public:
			CommandList();
			CommandList(CommandList&& rhs) noexcept				= delete;
			CommandList& operator=(CommandList&& rhs) noexcept	= delete;
			CommandList(CommandList const& rhs) 				= delete;
			CommandList& operator=(CommandList const& rhs) 		= delete;
			~CommandList();

			void begin();

			void end();

			// Ressource management:

			/**
			 * @brief 	Maps memory to a section of a staging buffer.
			 * 			The mapped memory will be copied to the specified buffer when the command list is executed.
			 * 			This means that this function can be used to virtually map memory of GPU_ONLY buffers.
			 * 			All mapped memory must be unmapped before submitting the command list to a queue.
			 * 
			 * @param copyDst is the buffer wich the mapped memory will be copied to.
			 * @param size size of the mapped memory.
			 * @param dstOffset offset into the buffer.
			 * @return MappedStagingMemory mapped memory.
			 */
			MappedStagingMemory mapMemoryStaged(BufferHandle copyDst, size_t size, size_t dstOffset);

			/**
			 * @brief Unmaps staging buffer memory.
			 * 
			 * @param mappedStagingMem mapped staging memory to unmap.
			 */
			void unmapMemoryStaged(MappedStagingMemory& mappedStagingMem);

			void copyHostToBuffer(HostToBufferCopyInfo copyInfo);

			void copyHostToImage(HostToImageCopyInfo copyInfo);

			void copyHostToImageSynced(HostToImageCopyInfo copyInfo, VkImageLayout finalImageLayout);

			void copyMultiBufferToBuffer(BufferToBufferMultiCopyInfo copyInfo);

			void copyBufferToBuffer(BufferToBufferCopyInfo copyInfo);

			void copyBufferToImage(BufferToImageCopyInfo copyInfo);

			// Rendering:

			/**
			 * Starts a render pass.
			*/
			void beginRendering(BeginRenderingInfo ri);

			/**
			 * Ends a render pass.
			*/
			void endRendering();

			/**
			 * Binds a pipeline to be used in the current render pass.
			 * 
			 * \param specify all render pass related information like render attachments.
			*/
			void bindPipeline(GraphicsPipelineHandle& graphicsPipeline);

			void setViewport(VkViewport const& viewport);

			void setScissor(VkRect2D const& scissor);

			template<typename T>
			void pushConstant(VkShaderStageFlagBits shaderStage, T& constant, size_t offset = 0) {
				vkCmdPushConstants(cmd, boundPipeline.value().layout, shaderStage, offset, sizeof(T), &constant);
			}

			void bindVertexBuffer(u32 binding, BufferHandle buffer, size_t bufferOffset = 0);

			void bindIndexBuffer(BufferHandle buffer, size_t bufferOffset = 0, VkIndexType indexType = VK_INDEX_TYPE_UINT32);

			void bindSet(u32 setBinding, BindingSetHandle set);

			void draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);

			void drawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstIntance);

			// sync:

			void insertBarriers(std::span<MemoryBarrier> memBarriers, std::span<BufferBarrier> bufBarriers, std::span<ImageBarrier> imgBarriers);

			void insertFullMemoryBarrier();

			// Accessors:

			VkCommandBuffer getVkCommandBuffer() { return cmd; }
			VkCommandPool getVkCommandPool() { return cmdPool; }
		private:
			friend class Device;
			friend class Queue;
			friend class CommandListHandle;

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
			u32 usesOnGPU = 0;				// counts in how many pending submits the command list is currently used
			bool empty = true;

			std::vector<BindingSetHandle> usedSets;
			std::vector<ImageHandle> usedImages;
			std::vector<BufferHandle> usedBuffers;
			std::vector<GraphicsPipelineHandle> usedGraphicsPipelines;
			VkDevice device;
			VkCommandBuffer cmd;
			VkCommandPool cmdPool;

			std::weak_ptr<CommandListRecyclingSharedData> recyclingData = {};
			std::weak_ptr<StagingBufferPool> stagingBufferPool = {};
			std::vector<StagingBuffer> usedStagingBuffers;
		};

		class CommandListHandle {
		public:
			CommandListHandle() = default;
			~CommandListHandle();

			CommandList& operator*() { return *list; }
			CommandList const& operator*() const { return *list; }
			CommandList* operator->() { return list.get(); }
			CommandList const* operator->() const { return list.get(); }

			operator bool() const { return list.operator bool(); }
			bool operator!() const { return !(list.operator bool()); }

			bool valid() const { return list.operator bool(); }
			
		private:
			friend class Device;
			friend class Queue;

			CommandListHandle(std::shared_ptr<CommandList> list);

			std::shared_ptr<CommandList> list = {};
		};
	}
}