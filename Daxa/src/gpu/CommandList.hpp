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

		struct BufferToImageCopy {
			size_t srcOffset = 0; 
			std::optional<VkImageSubresourceLayers> dstSubRessource = {};
			size_t size;
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
			~CommandList();

			CommandList(CommandList&& rhs) noexcept;
			CommandList& operator=(CommandList&& rhs) noexcept;

			CommandList(CommandList const& rhs) = delete;
			CommandList& operator=(CommandList const& rhs) = delete;

			void begin();

			void end();

			// Ressource management:

			/**
			 * Uploads from host memory to a staging buffer. 
			 * At the execution time of the command list, the data will be copied into the dst buffer from the staging buffer.
			 * 
			 * \param src host memory pointer.
			 * \param size size of the memory to upload in bytes.
			 * \param dst the buffer that the data will be uploaded to.
			 * \param dstOffset is a byte sized offset of where the uploaded memory will be placed in the host memory. 
			*/
			void uploadToBuffer(void const* src, size_t size, BufferHandle& dst, size_t dstOffset = 0);

			/**
			 * Uploads from host memory to a staging buffer.
			 * At the execution time of the command list, the data will be copied into the dst image from the staging buffer.
			 *
			 * \param src host memory pointer.
			 * \param size size of the memory to upload in bytes.
			 * \param dst the image that the data will be uploaded to.
			 * \param dstSubRessource gives an optional range defining to what part of the image the data is uploaded to.
			*/
			void uploadToImage(void const* src, size_t size, ImageHandle& dst, std::optional<VkImageSubresourceLayers> dstSubRessource = {});

			/**
			 * Uploads from host memory to a staging buffer.
			 * At the execution time of the command list, the data will be copied into the dst image from the staging buffer.
			 * this function insertes memory barriers by itself, wich may be a lot more inefficient manually syncing.
			 *
			 * \param src host memory pointer.
			 * \param size size of the memory to upload in bytes.
			 * \param dst the image that the data will be uploaded to.
			 * \param the layout of the image afte the upload.
			 * \param dstSubRessource gives an optional range defining to what part of the image the data is uploaded to.
			*/
			void uploadToImageSynced(void const* src, size_t size, ImageHandle& dst, VkImageLayout dstLayout, std::optional<VkImageSubresourceLayers> dstSubRessource = {});

			/**
			 * Copies the specified region from buffer src to buffer dst.
			 * 
			 * \param src the buffer that is copied from.
			 * \param dst the buffer that is copied to.
			 * \param copyInfo defines the src- and dst-offset and size of the memory to copy.
			*/
			void copyBufferToBuffer(BufferHandle& src, BufferHandle& dst, VkBufferCopy const& copyInfo) {
				copyBufferToBufferMulti(src, dst, { &copyInfo, 1 });
			}

			/**
			 * Copies the specified regions from buffer src to buffer dst.
			 *
			 * \param src the buffer that is copied from.
			 * \param dst the buffer that is copied to.
			 * \param copyInfos defines multiple ranges of values to be copied from src to dst.
			*/
			void copyBufferToBufferMulti(BufferHandle& src, BufferHandle& dst, std::span<VkBufferCopy const> copyInfos);

			// EXPERIMENTAL
			void copyBufferToImage(BufferHandle& src, ImageHandle& dst, BufferToImageCopy const& copyInfo);


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

			void bindVertexBuffer(u32 binding, BufferHandle& buffer, size_t bufferOffset = 0);

			void bindSet(u32 setBinding, BindingSetHandle& set);

			void draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);

			// sync:

			void insertBarriers(std::span<MemoryBarrier> memBarriers, std::span<BufferBarrier> bufBarriers, std::span<ImageBarrier> imgBarriers);

			// Accessors:

			VkCommandBuffer getVkCommandBuffer() { return cmd; }
			VkCommandPool getVkCommandPool() { return cmdPool; }
		private:
			friend class Device;
			friend class Queue;
			friend class CommandListHandle;

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