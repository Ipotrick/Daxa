#pragma once

#include "../DaxaCore.hpp"

#include <variant>
#include <span>
#include <vector>
#include <optional>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "Handle.hpp"
#include "Image.hpp"
#include "Buffer.hpp"
#include "SwapchainImage.hpp"
#include "Pipeline.hpp"
#include "BindingSet.hpp"
#include "StagingBufferPool.hpp"

namespace daxa {
	namespace gpu {

        // template<typename ValueT>
		// class MappedStagingMemory{
		// public:
		// 	MappedStagingMemory(void* hostPtr, size_t size, BufferHandle buffer)
		// 		: hostPtr{ hostPtr }
		// 		, size{ size }
		// 		, buffer{ std::move(buffer) }
		// 	{}
		// 	ValueT * const hostPtr;
		// 	size_t const size;
		// private:
		// 	friend class CommandList;
		// 	friend class Queue;
		// 	BufferHandle buffer = {};
		// };

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

		struct HostToImageCopySyncedInfo {
			void* src = nullptr;
			ImageHandle dst = {};
			std::optional<VkImageSubresourceLayers> dstImgSubressource = std::nullopt;
			size_t size = 0;
			VkImageLayout dstFinalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		};

		struct ImageToImageCopyInfo{
			ImageHandle src 			= {};
			VkImageLayout srcLayout 	= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			VkOffset3D srcOffset 		= {};
			ImageHandle dst 			= {};
			VkImageLayout dstLayout 	= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			VkOffset3D dstOffset 		= {};
			VkExtent3D size				= {};
		};

		struct ImageToImageCopySyncedInfo{
			ImageHandle src 						= {};
			VkOffset3D srcOffset 					= {};
			VkImageLayout srcLayoutBeforeAndAfter 	= {};
			ImageHandle dst 						= {};
			VkOffset3D dstOffset 					= {};
			VkImageLayout dstFinalLayout 			= {};
			VkExtent3D size							= {};
		};

		/**
		 * use this for all sync inside a queue:
		*/
		struct MemoryBarrier {
			VkPipelineStageFlags2KHR srcStages = VK_PIPELINE_STAGE_2_NONE_KHR;
			VkAccessFlags2KHR srcAccess = VK_ACCESS_2_NONE_KHR;
			VkPipelineStageFlags2KHR dstStages = VK_PIPELINE_STAGE_2_NONE_KHR;
			VkAccessFlags2KHR dstAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR;
		};

		static inline constexpr MemoryBarrier FULL_MEMORY_BARRIER = {
			.srcStages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR,
			.srcAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR | VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
			.dstStages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR,
			.dstAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR | VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
		};

		/**
		 * no real use case apparently:
		*/
		struct BufferBarrier {
			VkPipelineStageFlags2KHR srcStages = VK_PIPELINE_STAGE_2_NONE_KHR;
			VkAccessFlags2KHR srcAccess = VK_ACCESS_2_NONE_KHR;
			VkPipelineStageFlags2KHR dstStages = VK_PIPELINE_STAGE_2_NONE_KHR;
			VkAccessFlags2KHR dstAccess = VK_ACCESS_2_NONE_KHR;
			BufferHandle buffer = {};
			size_t offset = 0;
			std::optional<u32> size = {};
		};

		/**
		 * used for layout changes
		*/
		struct ImageBarrier {
			MemoryBarrier barrier = {};
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

			void finalize();

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
            template<typename ValueT = u8>
			MappedMemoryPointer<ValueT> mapMemoryStaged(BufferHandle copyDst, size_t size, size_t dstOffset) {
				auto ret = mapMemoryStagedVoid(copyDst, size, dstOffset);
				return { reinterpret_cast<ValueT*>(ret.hostPtr), ret.size, std::move(ret.owningBuffer) };
            }

			void copyHostToBuffer(HostToBufferCopyInfo copyInfo);

			void copyHostToImage(HostToImageCopyInfo copyInfo);

			void copyHostToImageSynced(HostToImageCopySyncedInfo copySyncedInfo);

			void copyMultiBufferToBuffer(BufferToBufferMultiCopyInfo copyInfo);

			void copyBufferToBuffer(BufferToBufferCopyInfo copyInfo);

			void copyBufferToImage(BufferToImageCopyInfo copyInfo);

			void copyImageToImage(ImageToImageCopyInfo copyInfo);

			void copyImageToImageSynced(ImageToImageCopySyncedInfo copySyncedInfo);

			// Compute:

			void dispatch(u32 groupCountX, u32 groupCountY = 1, u32 grpupCountZ = 1);

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
			void bindPipeline(PipelineHandle& pipeline);
			void unbindPipeline();

			void setViewport(VkViewport const& viewport);

			void setScissor(VkRect2D const& scissor);

			template<typename T>
			void pushConstant(VkShaderStageFlagBits shaderStage, T const& constant, size_t offset = 0) {
				vkCmdPushConstants(cmd, (**currentPipeline).getVkPipelineLayout(), shaderStage, offset, sizeof(T), &constant);
			}

			void bindVertexBuffer(u32 binding, BufferHandle buffer, size_t bufferOffset = 0);

			void bindIndexBuffer(BufferHandle buffer, size_t bufferOffset = 0, VkIndexType indexType = VK_INDEX_TYPE_UINT32);

			/**
			 * @brief 	Binds a binding set to the currently bound pipeline.
			 * 			the pipeline layout and binding point are infered by the currently bound pipeline.
			 * 
			 * @param setBinding set slot, the set should be bound to.
			 * @param set set that should be bound.
			 */
			void bindSet(u32 setBinding, BindingSetHandle set);

			/**
			 * @brief 	Binds a binding set to independantyl of the currently bound pipeline.
			 * 
			 * @param setBinding set slot, the set should be bound to.
			 * @param set set that should be bound.
			 * @param bindPoint bindPoint the set will be bound to.
			 * @param layout layout of the compatible pipelines.
			 */
			void bindSetPipelineIndependant(u32 setBinding, BindingSetHandle set, VkPipelineBindPoint bindPoint, VkPipelineLayout layout);

			void draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);

			void drawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstIntance);

			// sync:

			void insertBarriers(std::span<MemoryBarrier> memBarriers, std::span<ImageBarrier> imgBarriers);

			template<size_t N>
			void insertMemoryBarriers(std::array<MemoryBarrier, N> barriers) {
				insertBarriers({ barriers.data(), barriers.size()}, {});
			}

			void insertMemoryBarrier(MemoryBarrier barrier) {
				insertBarriers({ &barrier, 1}, {});
			}

			template<size_t N>
			void insertImageBarriers(std::array<ImageBarrier, N> barriers) {
				insertBarriers({}, { barriers.data(), barriers.size()});
			}

			void insertImageBarrier(ImageBarrier barrier) {
				insertBarriers({}, { &barrier, 1});
			}

			// Accessors:

			VkCommandBuffer getVkCommandBuffer() { return cmd; }
			VkCommandPool getVkCommandPool() { return cmdPool; }

			std::string const& getDebugName() const { return debugName; }
		private:
			friend class Device;
			friend class Queue;
			friend struct CommandListHandleStaticFunctionOverride;

			void reset();
			void begin();
			MappedMemoryPointer<u8> mapMemoryStagedVoid(BufferHandle copyDst, size_t size, size_t dstOffset);
			void setDebugName(char const* debugName);

			// binding set management:
			std::vector<VkDescriptorBufferInfo> bufferInfoBuffer = {};
			std::vector<VkDescriptorImageInfo> imageInfoBuffer = {};
			// begin rendering temporaries:
			std::vector<VkRenderingAttachmentInfoKHR> renderAttachmentBuffer = {};

			std::optional<PipelineHandle> currentPipeline = {};

			struct CurrentRenderPass{
				std::vector<RenderAttachmentInfo> colorAttachments 	= {};
				RenderAttachmentInfo* depthAttachment 				= nullptr;
				RenderAttachmentInfo* stencilAttachment 			= nullptr;
			};
			std::optional<CurrentRenderPass> currentRenderPass 		= {};

			/**
			 * @brief 	Checks if currently bound pipeline and render pass 'fit'
			 * 			A pipeline fits a renderpass, when all its needed attachemnts are present in the renderpass.
			 * 
			 */
			void checkIfPipelineAndRenderPassFit();

			void (*vkCmdBeginRenderingKHR)(VkCommandBuffer, const VkRenderingInfoKHR*);
			void (*vkCmdEndRenderingKHR)(VkCommandBuffer);
			void (*vkCmdPipelineBarrier2KHR)(VkCommandBuffer, VkDependencyInfoKHR const*);
			u32	operationsInProgress = 0;
			u32 usesOnGPU = 0;				// counts in how many pending submits the command list is currently used
			bool empty = true;
			bool finalized = false;

			std::vector<BindingSetHandle> usedSets = {};
			std::vector<ImageHandle> usedImages = {};
			std::vector<BufferHandle> usedBuffers = {};
			std::vector<PipelineHandle> usedGraphicsPipelines = {};
			VkDevice device = {};
			VkCommandBuffer cmd = {};
			VkCommandPool cmdPool = {};

			std::weak_ptr<CommandListRecyclingSharedData> recyclingData = {};
			std::weak_ptr<StagingBufferPool> stagingBufferPool = {};
			std::vector<StagingBuffer> usedStagingBuffers = {};

			std::string debugName = {};
		};

		struct CommandListHandleStaticFunctionOverride{
			static void cleanup(std::shared_ptr<CommandList>& value) {
				if (value && value.use_count() == 1) {
					if (auto recyclingSharedData = value->recyclingData.lock()) {
						value->reset();
						auto lock = std::unique_lock(recyclingSharedData->mut);
						recyclingSharedData->zombies.push_back(std::move(value));
					}
					value = {};
				}
			}
		};

		class CommandListHandle : public SharedHandle<CommandList, CommandListHandleStaticFunctionOverride>{};
	}
}