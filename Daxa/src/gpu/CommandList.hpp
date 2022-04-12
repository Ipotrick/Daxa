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
#include "Graveyard.hpp"
#include "TimelineSemaphore.hpp"

namespace daxa {
	static const VkPipelineStageFlagBits2KHR STAGE_NONE = 0ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_TOP_OF_PIPE = 0x00000001ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_DRAW_INDIRECT = 0x00000002ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_VERTEX_INPUT = 0x00000004ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_VERTEX_SHADER = 0x00000008ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_TESSELLATION_CONTROL_SHADER = 0x00000010ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_TESSELLATION_EVALUATION_SHADER = 0x00000020ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_GEOMETRY_SHADER = 0x00000040ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_FRAGMENT_SHADER = 0x00000080ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_EARLY_FRAGMENT_TESTS = 0x00000100ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_LATE_FRAGMENT_TESTS = 0x00000200ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_COLOR_ATTACHMENT_OUTPUT = 0x00000400ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_COMPUTE_SHADER = 0x00000800ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_ALL_TRANSFER = 0x00001000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_TRANSFER = 0x00001000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_BOTTOM_OF_PIPE = 0x00002000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_HOST = 0x00004000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_ALL_GRAPHICS = 0x00008000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_ALL_COMMANDS = 0x00010000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_COPY = 0x100000000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_RESOLVE = 0x200000000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_BLIT = 0x400000000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_CLEAR = 0x800000000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_INDEX_INPUT = 0x1000000000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_VERTEX_ATTRIBUTE_INPUT = 0x2000000000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_PRE_RASTERIZATION_SHADERS = 0x4000000000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_TRANSFORM_FEEDBACK_EXT = 0x01000000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_CONDITIONAL_RENDERING_EXT = 0x00040000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_COMMAND_PREPROCESS_NV = 0x00020000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT = 0x00400000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_SHADING_RATE_IMAGE_NV = 0x00400000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_ACCELERATION_STRUCTURE_BUILD = 0x02000000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_RAY_TRACING_SHADER = 0x00200000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_RAY_TRACING_SHADER_NV = 0x00200000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_ACCELERATION_STRUCTURE_BUILD_NV = 0x02000000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_FRAGMENT_DENSITY_PROCESS_EXT = 0x00800000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_TASK_SHADER_NV = 0x00080000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_MESH_SHADER_NV = 0x00100000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_SUBPASS_SHADING_HUAWEI = 0x8000000000ULL;
	static const VkPipelineStageFlagBits2KHR STAGE_INVOCATION_MASK_HUAWEI = 0x10000000000ULL;

	static const VkAccessFlagBits2KHR ACCESS_NONE = 0x00000000ULL;
	static const VkAccessFlagBits2KHR ACCESS_HOST_READ = 0x00002000ULL;
	static const VkAccessFlagBits2KHR ACCESS_HOST_WRITE = 0x00004000ULL;
	static const VkAccessFlagBits2KHR ACCESS_MEMORY_READ = 0x00008000ULL;
	static const VkAccessFlagBits2KHR ACCESS_MEMORY_WRITE = 0x00010000ULL;

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
		ImageViewHandle dst = {};
		size_t srcOffset = 0;
		std::optional<VkImageSubresourceLayers> subRessourceLayers = std::nullopt;
		size_t size = 0;
	};

	struct HostToImageCopyInfo {
		void* src = nullptr;
		ImageViewHandle dst = {};
		std::optional<VkImageSubresourceLayers> dstImgSubressource = std::nullopt;
		size_t size = 0;
	};

	struct ImageToImageCopyInfo{
		ImageViewHandle src 		= {};
		VkImageLayout srcLayout 	= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		VkOffset3D srcOffset 		= {};
		ImageViewHandle dst 		= {};
		VkImageLayout dstLayout 	= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		VkOffset3D dstOffset 		= {};
		VkExtent3D size				= {};
	};

	struct HostToBufferCopyRegion{
		u64 srcOffset = 0;
		u64 dstOffset = 0;
		u64 size = 0;
	};

	struct BufferToHostCopyRegion{
		u64 srcOffset = 0;
		u64 size = 0;
	};

	struct HostToImageCopyRegion{
		u64 srcOffset = 0;
		// defaulted to use layer 0 mip 0 color aspect
		VkImageSubresourceLayers subRessource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};
		// defaulted to use no offset
		VkOffset3D imageOffset = {  0,  0,  0 };
		// defaulted to use the full image extent
		VkExtent3D imageExtent = { 0, 0, 0 };
	};

	/// MUST BE ABI COMPATIBLE TO VkBufferCopy
	struct BufferToBufferCopyRegion{
		u64 srcOffset = 0;
		u64 dstOffset = 0;
		u64 size = 0;
	};

	/// MUST BE ABI COMPATIBLE TO VkBufferImageCopy
	struct BufferToImageCopyRegion{
		u64 bufferOffset = 0;
		u32 bufferRowLength = 0;
		u32 bufferImageHeight = 0;
		// defaulted to use layer 0 mip 0 color aspect
		VkImageSubresourceLayers subRessource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};
		// defaulted to use no offset
		VkOffset3D imageOffset = {  0,  0,  0 };
		// defaulted to use the full image extent
		VkExtent3D imageExtent = { 0, 0, 0 };
	};

	struct ImageToImageCopyRegion{
		VkImageSubresourceLayers    srcSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};
		VkOffset3D                  srcOffset 		= { 0, 0, 0, };
		VkImageSubresourceLayers    dstSubresource 	= {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};
		VkOffset3D                  dstOffset 		= { 0, 0, 0, };
		VkExtent3D                  extent 			= { 0, 0, 0, };
	};

	/**
	 * use this for all sync inside a queue:
	*/
	struct MemoryBarrier {
		VkPipelineStageFlags2KHR srcStages = STAGE_NONE;
		VkAccessFlags2KHR srcAccess = ACCESS_NONE;
		VkPipelineStageFlags2KHR dstStages = STAGE_NONE;
		VkAccessFlags2KHR dstAccess = ACCESS_MEMORY_READ;
	};

	static inline constexpr MemoryBarrier FULL_MEMORY_BARRIER = {
		.srcStages = STAGE_ALL_COMMANDS,
		.srcAccess = ACCESS_MEMORY_READ | ACCESS_MEMORY_WRITE,
		.dstStages = STAGE_ALL_COMMANDS,
		.dstAccess = ACCESS_MEMORY_READ | ACCESS_MEMORY_WRITE,
	};

	/**
	 * used for layout changes
	*/
	struct ImageBarrier {
		MemoryBarrier 							barrier 		= FULL_MEMORY_BARRIER;
		ImageViewHandle 						image 			= {};
		VkImageLayout 							layoutBefore	= {};
		VkImageLayout 							layoutAfter		= {}; 
		u32 									srcQueueIndex 	= VK_QUEUE_FAMILY_IGNORED;
		u32 									dstQueueIndex 	= VK_QUEUE_FAMILY_IGNORED;
		// this default is invalid and will be replaced by the full range of the view if not overwritten
		VkImageSubresourceRange 				subRange 		= { .aspectMask = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM, };
	};

	struct RenderAttachmentInfo {
		ImageViewHandle image = {};
		VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkResolveModeFlagBits resolveMode = VK_RESOLVE_MODE_NONE;
		VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
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
		MappedMemoryPointer mapMemoryStagedBuffer(BufferHandle copyDst, size_t size, size_t dstOffset) {
			auto ret = mapMemoryStagedBufferVoid(copyDst, size, dstOffset);
			return { reinterpret_cast<u8*>(ret.hostPtr), ret.size, std::move(ret.owningBuffer) };
		}
		
		MappedMemoryPointer mapMemoryStagedImage(
			ImageHandle copyDst, 
			VkImageSubresourceLayers subressource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 }, 
			VkOffset3D dstOffset = { 0, 0, 0 }, 
			VkExtent3D dstExtent = { 0, 0, 0 }
		) {
			auto ret = mapMemoryStagedImageVoid(copyDst, subressource, dstOffset, dstExtent);
			return { reinterpret_cast<u8*>(ret.hostPtr), ret.size, std::move(ret.owningBuffer) };
		}

		//void copyHostToBuffer2(BufferHandle& srcBuffer, ImageViewHandle& dstImage, std::span<VkBufferImageCopy> regions = {});
//
		//void copyHostToBuffer(HostToBufferCopyInfo copyInfo);
//
		//void copyHostToImage(HostToImageCopyInfo copyInfo);
//
		//void copyMultiBufferToBuffer(BufferToBufferMultiCopyInfo copyInfo);
//
		//void copyBufferToBuffer(BufferToBufferCopyInfo copyInfo);
//
		//void copyBufferToImage(BufferToImageCopyInfo copyInfo);
//
		//void copyImageToImage(ImageToImageCopyInfo copyInfo);

		// transfer2:

		struct MultiHostToBufferCopyInfo{
			u8 const* src; 
			BufferHandle const& dst;
			std::span<HostToBufferCopyRegion const> regions;
		};
		void multiCopyHostToBuffer(MultiHostToBufferCopyInfo const& ci);

		struct SingleCopyHostToBufferInfo{
			u8 const* src; 
			BufferHandle const& dst;
			HostToBufferCopyRegion region = {};
		};
		void singleCopyHostToBuffer(SingleCopyHostToBufferInfo const& info);

		

		struct ToHostCopyFuture{
		public:
			bool ready() const {
				return timelineHandle->getCounter() == timelineReadyValue;
			}

			operator bool() const { 
				return ready();
			}

			bool operator !() const { 
				return !ready();
			}
			
			std::optional<MappedMemoryPointer> getPtr() {
				if (ready()) {
					MappedMemoryPointer mem = readBackStagingBuffer.buffer->mapMemory();
					mem.hostPtr = static_cast<u8*>(mem.hostPtr) + readBackBufferOffset;
					return std::optional<MappedMemoryPointer>{ std::move(mem) };
				}
				return std::nullopt;
			}
		private:
			friend class CommandList;
			ToHostCopyFuture(
				StagingBuffer&& readBackStagingBuffer,
				TimelineSemaphoreHandle timelineHandle,
				u64 timelineReadyValue,
				u64 readBackBufferOffset
			)
				: readBackStagingBuffer{ std::move(readBackStagingBuffer) }
				, timelineHandle{ timelineHandle }
				, timelineReadyValue{ timelineReadyValue }
				, readBackBufferOffset{ readBackBufferOffset }
			{}

			StagingBuffer readBackStagingBuffer;
			TimelineSemaphoreHandle timelineHandle = {};
			u64 timelineReadyValue = {};
			u64 readBackBufferOffset = {};
		};
		struct SingleBufferToHostCopyInfo{
			BufferHandle const& src;
			BufferToHostCopyRegion region = {};
		};
		ToHostCopyFuture singleCopyBufferToHost(SingleBufferToHostCopyInfo const& info);



		struct MultiHostToImageCopyInfo{
			u8 const* src;
			ImageHandle const& dst;
			VkImageLayout dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			std::span<HostToImageCopyRegion const> regions;
		};
		void multiCopyHostToImage(MultiHostToImageCopyInfo const& ci);

		struct SingleHostToImageCopyInfo{
			u8 const* src;
			ImageHandle const& dst;
			VkImageLayout dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			HostToImageCopyRegion region = {};
		};
		void singleCopyHostToImage(SingleHostToImageCopyInfo const& ci);



		struct MultiBufferToBufferCopyInfo{
			BufferHandle const& src;
			BufferHandle const& dst;
			std::span<BufferToBufferCopyRegion const> regions;
		};
		void multiCopyBufferToBuffer(MultiBufferToBufferCopyInfo const& ci);

		struct SingleBufferToBufferCopyInfo{
			BufferHandle const& src;
			BufferHandle const& dst;
			BufferToBufferCopyRegion region = {};
		};
		void singleCopyBufferToBuffer(SingleBufferToBufferCopyInfo const& ci);



		struct MultiBufferToImageCopyInfo{
			BufferHandle const& src;
			ImageHandle const& dst;
			VkImageLayout dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			std::span<BufferToImageCopyRegion const> regions;
		};
		void multiCopyBufferToImage(MultiBufferToImageCopyInfo const& ci);

		struct SingleBufferToImageCopyInfo{
			BufferHandle const& src;
			ImageHandle const& dst;
			VkImageLayout dstLayout;
			BufferToImageCopyRegion region = {};
		};
		void singleCopyBufferToImage(SingleBufferToImageCopyInfo const& ci);



		struct MultiImageToImageCopyInfo{
			ImageHandle const& src;
			VkImageLayout srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			ImageHandle const& dst;
			VkImageLayout dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			std::span<ImageToImageCopyRegion const> regions;
		};
		void multiCopyImageToImage(MultiImageToImageCopyInfo const& regions);

		struct SingleImageToImageCopyInfo{
			ImageHandle const& src;
			VkImageLayout srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			ImageHandle const& dst;
			VkImageLayout dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			ImageToImageCopyRegion region = {};
		};
		void singleCopyImageToImage(SingleImageToImageCopyInfo const& ci);

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
		void pushConstant(VkShaderStageFlags shaderStage, T const& constant, size_t offset = 0) {
			vkCmdPushConstants(cmd, (**currentPipeline).getVkPipelineLayout(), shaderStage, static_cast<u32>(offset), static_cast<u32>(sizeof(T)), &constant);
		}

		void bindVertexBuffer(u32 binding, BufferHandle& buffer, size_t bufferOffset = 0);

		void bindIndexBuffer(BufferHandle& buffer, size_t bufferOffset = 0, VkIndexType indexType = VK_INDEX_TYPE_UINT32);

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

		void queueMemoryBarrier(MemoryBarrier const& memoryBarrier);
		void queueImageBarrier(ImageBarrier const& memoryBarrier);
		void insertQueuedBarriers();

		// Bindless Descriptors:

		void bindAll(u32 set = 0);

		// Accessors:

		VkCommandBuffer getVkCommandBuffer() { return cmd; }
		VkCommandPool getVkCommandPool() { return cmdPool; }

		std::string const& getDebugName() const { return debugName; }
	private:
		friend class Device;
		friend class CommandQueue;
		friend struct CommandListHandleStaticFunctionOverride;

		void reset();
		void begin();
		void setDebugName(char const* debugName);
		void checkIfPipelineAndRenderPassFit();
		MappedMemoryPointer mapMemoryStagedBufferVoid(BufferHandle copyDst, size_t size, size_t dstOffset);
		MappedMemoryPointer mapMemoryStagedImageVoid(ImageHandle copyDst, VkImageSubresourceLayers subRessource, VkOffset3D dstOffset, VkExtent3D dstExtent);

		struct CurrentRenderPass{
			std::vector<RenderAttachmentInfo> 		colorAttachments 	= {};
			std::optional<RenderAttachmentInfo> 	depthAttachment 	= {};
			std::optional<RenderAttachmentInfo> 	stencilAttachment 	= {};
		};

		bool 											bBarriersQueued 			= {};
		std::vector<VkMemoryBarrier2KHR>				queuedMemoryBarriers		= {};
		std::vector<VkBufferMemoryBarrier2KHR>			queuedBufferBarriers		= {};
		std::vector<VkImageMemoryBarrier2KHR>			queuedImageBarriers			= {};
		std::shared_ptr<DeviceBackend> 					deviceBackend				= {};
		VkCommandBuffer 								cmd 						= {};
		VkCommandPool 									cmdPool 					= {};
		std::vector<VkDescriptorBufferInfo> 			bufferInfoBuffer 			= {};
		std::vector<VkDescriptorImageInfo> 				imageInfoBuffer 			= {};
		std::vector<VkBufferImageCopy>					bufferImageCopyBuffer 		= {};
		std::vector<VkRenderingAttachmentInfoKHR> 		renderAttachmentBuffer 		= {};
		std::optional<PipelineHandle> 					currentPipeline 			= {};
		std::optional<CurrentRenderPass> 				currentRenderPass 			= {};
		u32												operationsInProgress 		= 0;
		u32 											usesOnGPU 					= 0;				// counts in how many pending submits the command list is currently used
		bool 											empty 						= true;
		bool 											finalized 					= false;
		std::vector<BindingSetHandle> 					usedSets 					= {};
		std::vector<PipelineHandle> 					usedGraphicsPipelines 		= {};
		std::weak_ptr<CommandListRecyclingSharedData> 	recyclingData 				= {};
		std::weak_ptr<StagingBufferPool> 				uploadStagingBufferPool 	= {};
		std::vector<StagingBuffer> 						usedUploadStagingBuffers 	= {};
		std::weak_ptr<StagingBufferPool>				downloadStagingBufferPool 	= {};
		std::vector<StagingBuffer> 						usedDownloadStagingBuffers 	= {};
		std::vector<std::pair<TimelineSemaphoreHandle,u64>>			
														usedTimelines 				= {};
		std::shared_ptr<ZombieList> 					zombies 					= std::make_shared<ZombieList>();
		std::string 									debugName 					= {};
	};

	struct CommandListHandleStaticFunctionOverride{
		static void cleanup(std::shared_ptr<CommandList>& value) {
			if (value && value.use_count() == 1) {
				if (auto recyclingSharedData = value->recyclingData.lock()) {
					value->reset();
					auto lock = std::unique_lock(recyclingSharedData->mut);
					std::shared_ptr<CommandList> dummy{};
					std::swap(dummy, value);
					recyclingSharedData->zombies.push_back(std::move(dummy));
				}
				value = {};
			}
		}
	};

	class CommandListHandle : public SharedHandle<CommandList, CommandListHandleStaticFunctionOverride>{};
}