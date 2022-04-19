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
	static const u64 STAGE_NONE = 0ULL;
	static const u64 STAGE_TOP_OF_PIPE = 0x00000001ULL;
	static const u64 STAGE_DRAW_INDIRECT = 0x00000002ULL;
	static const u64 STAGE_VERTEX_INPUT = 0x00000004ULL;
	static const u64 STAGE_VERTEX_SHADER = 0x00000008ULL;
	static const u64 STAGE_TESSELLATION_CONTROL_SHADER = 0x00000010ULL;
	static const u64 STAGE_TESSELLATION_EVALUATION_SHADER = 0x00000020ULL;
	static const u64 STAGE_GEOMETRY_SHADER = 0x00000040ULL;
	static const u64 STAGE_FRAGMENT_SHADER = 0x00000080ULL;
	static const u64 STAGE_EARLY_FRAGMENT_TESTS = 0x00000100ULL;
	static const u64 STAGE_LATE_FRAGMENT_TESTS = 0x00000200ULL;
	static const u64 STAGE_COLOR_ATTACHMENT_OUTPUT = 0x00000400ULL;
	static const u64 STAGE_COMPUTE_SHADER = 0x00000800ULL;
	static const u64 STAGE_ALL_TRANSFER = 0x00001000ULL;
	static const u64 STAGE_TRANSFER = 0x00001000ULL;
	static const u64 STAGE_BOTTOM_OF_PIPE = 0x00002000ULL;
	static const u64 STAGE_HOST = 0x00004000ULL;
	static const u64 STAGE_ALL_GRAPHICS = 0x00008000ULL;
	static const u64 STAGE_ALL_COMMANDS = 0x00010000ULL;
	static const u64 STAGE_COPY = 0x100000000ULL;
	static const u64 STAGE_RESOLVE = 0x200000000ULL;
	static const u64 STAGE_BLIT = 0x400000000ULL;
	static const u64 STAGE_CLEAR = 0x800000000ULL;
	static const u64 STAGE_INDEX_INPUT = 0x1000000000ULL;
	static const u64 STAGE_VERTEX_ATTRIBUTE_INPUT = 0x2000000000ULL;
	static const u64 STAGE_PRE_RASTERIZATION_SHADERS = 0x4000000000ULL;
	static const u64 STAGE_TRANSFORM_FEEDBACK_EXT = 0x01000000ULL;
	static const u64 STAGE_CONDITIONAL_RENDERING_EXT = 0x00040000ULL;
	static const u64 STAGE_COMMAND_PREPROCESS_NV = 0x00020000ULL;
	static const u64 STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT = 0x00400000ULL;
	static const u64 STAGE_SHADING_RATE_IMAGE_NV = 0x00400000ULL;
	static const u64 STAGE_ACCELERATION_STRUCTURE_BUILD = 0x02000000ULL;
	static const u64 STAGE_RAY_TRACING_SHADER = 0x00200000ULL;
	static const u64 STAGE_RAY_TRACING_SHADER_NV = 0x00200000ULL;
	static const u64 STAGE_ACCELERATION_STRUCTURE_BUILD_NV = 0x02000000ULL;
	static const u64 STAGE_FRAGMENT_DENSITY_PROCESS_EXT = 0x00800000ULL;
	static const u64 STAGE_TASK_SHADER_NV = 0x00080000ULL;
	static const u64 STAGE_MESH_SHADER_NV = 0x00100000ULL;
	static const u64 STAGE_SUBPASS_SHADING_HUAWEI = 0x8000000000ULL;
	static const u64 STAGE_INVOCATION_MASK_HUAWEI = 0x10000000000ULL;

	static const u64 ACCESS_NONE = 0x00000000ULL;
	static const u64 ACCESS_HOST_READ = 0x00002000ULL;
	static const u64 ACCESS_HOST_WRITE = 0x00004000ULL;
	static const u64 ACCESS_MEMORY_READ = 0x00008000ULL;
	static const u64 ACCESS_MEMORY_WRITE = 0x00010000ULL;

	enum class ImageAspect : u64{
		COLOR_BIT = 0x00000001,
		DEPTH_BIT = 0x00000002,
		STENCIL_BIT = 0x00000004,
		METADATA_BIT = 0x00000008,
		PLANE_0_BIT = 0x00000010,
		PLANE_1_BIT = 0x00000020,
		PLANE_2_BIT = 0x00000040,
		MEMORY_PLANE_0_BIT_EXT = 0x00000080,
		MEMORY_PLANE_1_BIT_EXT = 0x00000100,
		MEMORY_PLANE_2_BIT_EXT = 0x00000200,
		MEMORY_PLANE_3_BIT_EXT = 0x00000400,
		PLANE_0_BIT_KHR = VK_IMAGE_ASPECT_PLANE_0_BIT,
		PLANE_1_BIT_KHR = VK_IMAGE_ASPECT_PLANE_1_BIT,
		PLANE_2_BIT_KHR = VK_IMAGE_ASPECT_PLANE_2_BIT,
	};

	struct ImageLayers {
		ImageAspect	aspectMask;
		u32	 		mipLevel;
		u32 		baseArrayLayer;
		u32 		layerCount;
	};

	struct ImageMipLayers {
		ImageAspect aspectMask;
		u32 		mipLevel;
		u32 		levelCount;
		u32 		baseArrayLayer;
		u32 		layerCount;
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
		friend class CommandListBackend;
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
	
	class CommandListBackend;

	struct CommandListRecyclingSharedData {
		std::mutex mut = {};
		std::vector<std::shared_ptr<CommandListBackend>> zombies;
	};

	struct MultiHostToBufferCopyInfo{
		u8 const* src; 
		BufferHandle const& dst;
		std::span<HostToBufferCopyRegion const> regions;
	};

	struct SingleCopyHostToBufferInfo{
		u8 const* src; 
		BufferHandle const& dst;
		HostToBufferCopyRegion region = {};
	};

	struct MultiHostToImageCopyInfo{
		u8 const* src;
		ImageHandle const& dst;
		VkImageLayout dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		std::span<HostToImageCopyRegion const> regions;
	};

	struct SingleHostToImageCopyInfo{
		u8 const* src;
		ImageHandle const& dst;
		VkImageLayout dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		HostToImageCopyRegion region = {};
	};

	struct MultiBufferToBufferCopyInfo{
		BufferHandle const& src;
		BufferHandle const& dst;
		std::span<BufferToBufferCopyRegion const> regions;
	};

	struct SingleBufferToBufferCopyInfo{
		BufferHandle const& src;
		BufferHandle const& dst;
		BufferToBufferCopyRegion region = {};
	};

	struct MultiBufferToImageCopyInfo{
		BufferHandle const& src;
		ImageHandle const& dst;
		VkImageLayout dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		std::span<BufferToImageCopyRegion const> regions;
	};

	struct SingleBufferToImageCopyInfo{
		BufferHandle const& src;
		ImageHandle const& dst;
		VkImageLayout dstLayout;
		BufferToImageCopyRegion region = {};
	};

	struct MultiImageToImageCopyInfo{
		ImageHandle const& src;
		VkImageLayout srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		ImageHandle const& dst;
		VkImageLayout dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		std::span<ImageToImageCopyRegion const> regions;
	};

	struct SingleImageToImageCopyInfo{
		ImageHandle const& src;
		VkImageLayout srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		ImageHandle const& dst;
		VkImageLayout dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		ImageToImageCopyRegion region = {};
	};

	struct CommandListHandleStaticFunctionOverride{
		static void cleanup(std::shared_ptr<CommandListBackend>& value);
	};

	struct CommandListDeadEnd {

	};

	class CommandListHandle : public SharedHandle<CommandListBackend, CommandListHandleStaticFunctionOverride>{
	public:
		CommandListDeadEnd operator*() const { return CommandListDeadEnd{}; }
		CommandListDeadEnd operator*() { return CommandListDeadEnd{}; }
		CommandListDeadEnd operator->() const { return CommandListDeadEnd{}; }
		CommandListDeadEnd operator->() { return CommandListDeadEnd{}; }

		void finalize();
		MappedMemoryPointer mapMemoryStagedBuffer(BufferHandle copyDst, size_t size, size_t dstOffset);
		MappedMemoryPointer mapMemoryStagedImage(
			ImageHandle copyDst, 
			VkImageSubresourceLayers subressource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 }, 
			VkOffset3D dstOffset = { 0, 0, 0 }, 
			VkExtent3D dstExtent = { 0, 0, 0 }
		);
		void multiCopyHostToBuffer(MultiHostToBufferCopyInfo const& ci);
		void singleCopyHostToBuffer(SingleCopyHostToBufferInfo const& info);
		ToHostCopyFuture singleCopyBufferToHost(SingleBufferToHostCopyInfo const& ci);
		void multiCopyHostToImage(MultiHostToImageCopyInfo const& ci);
		void singleCopyHostToImage(SingleHostToImageCopyInfo const& ci);
		void multiCopyBufferToBuffer(MultiBufferToBufferCopyInfo const& ci);
		void singleCopyBufferToBuffer(SingleBufferToBufferCopyInfo const& ci);
		void multiCopyBufferToImage(MultiBufferToImageCopyInfo const& ci);
		void singleCopyBufferToImage(SingleBufferToImageCopyInfo const& ci);
		void multiCopyImageToImage(MultiImageToImageCopyInfo const& ci);
		void singleCopyImageToImage(SingleImageToImageCopyInfo const& ci);
		void dispatch(u32 groupCountX, u32 groupCountY = 1, u32 grpupCountZ = 1);
		void beginRendering(BeginRenderingInfo ri);
		void endRendering();
		void bindPipeline(PipelineHandle& pipeline);
		void unbindPipeline();
		void setViewport(VkViewport const& viewport);
		void setScissor(VkRect2D const& scissor);
		template<typename T>
		void pushConstant(VkShaderStageFlags shaderStage, T const& constant, size_t offset = 0) {
			pushConstant(shaderStage, &constant, static_cast<u32>(sizeof(T)), static_cast<u32>(offset));
		}
		void pushConstant(VkShaderStageFlags shaderStage, void const* data, u32 size, u32 offset = 0);
		void bindVertexBuffer(u32 binding, BufferHandle& buffer, size_t bufferOffset = 0);
		void bindIndexBuffer(BufferHandle& buffer, size_t bufferOffset = 0, VkIndexType indexType = VK_INDEX_TYPE_UINT32);
		void bindSet(u32 setBinding, BindingSetHandle set);
		void bindSetPipelineIndependant(u32 setBinding, BindingSetHandle set, VkPipelineBindPoint bindPoint, VkPipelineLayout layout);
		void draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);
		void drawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstIntance);
		void queueMemoryBarrier(MemoryBarrier const& memoryBarrier);
		void queueImageBarrier(ImageBarrier const& memoryBarrier);
		void insertQueuedBarriers();
		void bindAll(u32 set = 0);
		VkCommandBuffer getVkCommandBuffer();
		VkCommandPool getVkCommandPool();
		std::string const& getDebugName() const;
	};
}