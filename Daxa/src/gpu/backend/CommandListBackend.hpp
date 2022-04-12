#include "DeviceBackend.hpp"
#include "../CommandList.hpp"

namespace daxa {
    class CommandListBackend {
	public:
		CommandListBackend();
		CommandListBackend(CommandListBackend&& rhs) noexcept				= delete;
		CommandListBackend& operator=(CommandListBackend&& rhs) noexcept	= delete;
		CommandListBackend(CommandListBackend const& rhs) 				= delete;
		CommandListBackend& operator=(CommandListBackend const& rhs) 		= delete;
		~CommandListBackend();

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

		ToHostCopyFuture singleCopyBufferToHost(SingleBufferToHostCopyInfo const& info);

		void multiCopyHostToImage(MultiHostToImageCopyInfo const& ci);

		void singleCopyHostToImage(SingleHostToImageCopyInfo const& ci);

		void multiCopyBufferToBuffer(MultiBufferToBufferCopyInfo const& ci);

		void singleCopyBufferToBuffer(SingleBufferToBufferCopyInfo const& ci);

		void multiCopyBufferToImage(MultiBufferToImageCopyInfo const& ci);

		void singleCopyBufferToImage(SingleBufferToImageCopyInfo const& ci);

		void multiCopyImageToImage(MultiImageToImageCopyInfo const& regions);
		void singleCopyImageToImage(SingleImageToImageCopyInfo const& ci);

		void dispatch(u32 groupCountX, u32 groupCountY = 1, u32 grpupCountZ = 1);

		void beginRendering(BeginRenderingInfo ri);

		void endRendering();

		void bindPipeline(PipelineHandle& pipeline);
		void unbindPipeline();

		void setViewport(VkViewport const& viewport);

		void setScissor(VkRect2D const& scissor);

		//template<typename T>
		//void pushConstant(VkShaderStageFlags shaderStage, T const& constant, u32 offset = 0) {
		//	vkCmdPushConstants(cmd, (**currentPipeline).getVkPipelineLayout(), shaderStage, static_cast<u32>(offset), static_cast<u32>(sizeof(T)), &constant);
		//}

		void pushConstant(VkShaderStageFlags shaderStage, void const* daxa, u32 size, u32 offset = 0);

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
}