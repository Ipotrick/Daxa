#include "CommandListBackend.hpp"

#include "../util.hpp"
#include "../Instance.hpp"

namespace daxa {
    size_t byteSizeOfImageRange(ImageHandle const& image, VkImageSubresourceLayers subRessource, VkOffset3D offset, VkExtent3D extent) {
		return sizeofFormat(image->getVkFormat())
			* subRessource.layerCount
			* (extent.width - offset.x)
			* (extent.height - offset.y)
			* (extent.depth - offset.z);
	}

	size_t roundUpToMultipleOf128(size_t n) {
		size_t remainder = n % 128;
		n -= remainder;
		n += 128 * size_t(bool(remainder));
		return n;
	}

	CommandListBackend::CommandListBackend() {
		this->renderAttachmentBuffer.reserve(5);
		usedSets.reserve(10);
		usedGraphicsPipelines.reserve(3);
		usedUploadStagingBuffers.reserve(1);
	}

	CommandListBackend::~CommandListBackend() {
		if (deviceBackend) {
			DAXA_ASSERT_M(operationsInProgress == 0, "a command list can not be descroyed when there are still commands recorded");
			DAXA_ASSERT_M(empty, "a command list can not be destroyed when not empty");
			vkFreeCommandBuffers(deviceBackend->device.device, cmdPool, 1, &cmd);
			vkDestroyCommandPool(deviceBackend->device.device, cmdPool, nullptr);
			deviceBackend = VK_NULL_HANDLE;
		}
	}

	void CommandListBackend::finalize() {
		if (bBarriersQueued) { insertQueuedBarriers(); }
		DAXA_ASSERT_M(finalized == false, "can not finalize a command list twice");
		DAXA_ASSERT_M(operationsInProgress == 0, "can only finalize a command list that has no operations in progress");
		DAXA_CHECK_VK_RESULT(vkEndCommandBuffer(cmd));
		finalized = true;
	}

	MappedMemoryPointer CommandListBackend::mapMemoryStagedBufferVoid(BufferHandle copyDst, size_t size, size_t dstOffset) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(size <= STAGING_BUFFER_POOL_BUFFER_SIZE, "Currently uploads over a size of 67.108.864 bytes are not supported by the uploadToBuffer function. Please use a staging buffer.");
		if (bBarriersQueued) { insertQueuedBarriers(); }
		
		if (usedUploadStagingBuffers.empty() || usedUploadStagingBuffers.back().getLeftOverSize() < size) {
			usedUploadStagingBuffers.push_back(uploadStagingBufferPool.lock()->getStagingBuffer());
		}

		auto& stagingBuffer = usedUploadStagingBuffers.back();

		auto srcOffset = stagingBuffer.usedUpSize;

		auto mm = stagingBuffer.buffer->mapMemory();

		mm.hostPtr += srcOffset;
		mm.size = size;

		stagingBuffer.usedUpSize += size;
		stagingBuffer.usedUpSize = roundUpToMultipleOf128(stagingBuffer.usedUpSize);

		singleCopyBufferToBuffer({
			.src = *stagingBuffer.buffer,
			.dst = copyDst,
			.region = {
				.srcOffset = srcOffset,
				.dstOffset = dstOffset,
				.size = size,
			}
		});
		
		return mm;
	}

	MappedMemoryPointer CommandListBackend::mapMemoryStagedImageVoid(ImageHandle copyDst, VkImageSubresourceLayers subRessource, VkOffset3D dstOffset, VkExtent3D dstExtent) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		if (bBarriersQueued) { insertQueuedBarriers(); }
		if (dstExtent.width == 0) {
			dstExtent.width = copyDst->getVkExtent3D().width;
		}
		if (dstExtent.height == 0) {
			dstExtent.height = copyDst->getVkExtent3D().height;
		}
		if (dstExtent.depth == 0) {
			dstExtent.depth = copyDst->getVkExtent3D().depth;
		}
		auto size = byteSizeOfImageRange(copyDst, subRessource, dstOffset, dstExtent);
		DAXA_ASSERT_M(size <= STAGING_BUFFER_POOL_BUFFER_SIZE, "Currently uploads over a size of 67.108.864 bytes are not supported by the uploadToBuffer function. Please use a staging buffer.");
		
		if (usedUploadStagingBuffers.empty() || usedUploadStagingBuffers.back().getLeftOverSize() < size) {
			usedUploadStagingBuffers.push_back(uploadStagingBufferPool.lock()->getStagingBuffer());
		}

		auto& stagingBuffer = usedUploadStagingBuffers.back();
		
		auto srcOffset = stagingBuffer.usedUpSize;

		auto mm = stagingBuffer.buffer->mapMemory();

		mm.hostPtr += srcOffset;
		mm.size = size;

		stagingBuffer.usedUpSize += size;
		stagingBuffer.usedUpSize = roundUpToMultipleOf128(stagingBuffer.usedUpSize);

		singleCopyBufferToImage({
			.src = *stagingBuffer.buffer,
			.dst = copyDst,
			.dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.region = {
				.bufferOffset = srcOffset,
				.subRessource = subRessource,
				.imageOffset = dstOffset,
				.imageExtent = dstExtent,
			}
		});

		return mm;
	}
	

	///
	/// 	TRANSFER 2 BEGIN
	///
	
	MappedMemoryPointer CommandListBackend::mapMemoryStagedImage(
		ImageHandle copyDst, 
		VkImageSubresourceLayers subressource, 
		VkOffset3D dstOffset, 
		VkExtent3D dstExtent
	) {
		auto ret = mapMemoryStagedImageVoid(copyDst, subressource, dstOffset, dstExtent);
		return { reinterpret_cast<u8*>(ret.hostPtr), ret.size, std::move(ret.owningBuffer) };
	}
	
	MappedMemoryPointer CommandListBackend::mapMemoryStagedBuffer(BufferHandle copyDst, size_t size, size_t dstOffset) {
		auto ret = mapMemoryStagedBufferVoid(copyDst, size, dstOffset);
		return { reinterpret_cast<u8*>(ret.hostPtr), ret.size, std::move(ret.owningBuffer) };
	}


	void CommandListBackend::multiCopyHostToBuffer(MultiHostToBufferCopyInfo const& ci) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(!currentRenderPass.has_value(), "can not record copy commands in a render pass");
		if (bBarriersQueued) { insertQueuedBarriers(); }

		for (size_t i = 0; i < ci.regions.size(); i++) {
			auto const& info = ci.regions[i];
			auto mappedmem = this->mapMemoryStagedBuffer(ci.dst, info.size, info.dstOffset);
			std::memcpy(mappedmem.hostPtr, ci.src + info.srcOffset, info.size);
		}
	}
	void CommandListBackend::singleCopyHostToBuffer(SingleCopyHostToBufferInfo const& ci) {
		this->multiCopyHostToBuffer({ .src = ci.src, .dst = ci.dst, .regions = std::span{&ci.region, 1} });
	}


	
	ToHostCopyFuture CommandListBackend::singleCopyBufferToHost(SingleBufferToHostCopyInfo const& info) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(!currentRenderPass.has_value(), "can not record copy commands in a render pass");
		if (bBarriersQueued) { insertQueuedBarriers(); }

		if (usedDownloadStagingBuffers.empty() || usedDownloadStagingBuffers.back().getLeftOverSize() < info.region.size) {
			usedDownloadStagingBuffers.push_back(uploadStagingBufferPool.lock()->getStagingBuffer());
		}

		auto& stagingBuffer = usedUploadStagingBuffers.back();
		
		auto srcOffset = stagingBuffer.usedUpSize;

		auto timeline = TimelineSemaphoreHandle{ 
			std::make_shared<TimelineSemaphore>(
				deviceBackend, 
				TimelineSemaphoreCreateInfo{
					.initialValue = 0,
					.debugName = "read back timeline semaphore",
				}
			)
		};
		u64 readyValue = 1;

		this->usedTimelines.push_back({timeline, readyValue});

		stagingBuffer.usedUpSize += info.region.size;
		stagingBuffer.usedUpSize = roundUpToMultipleOf128(stagingBuffer.usedUpSize);

		u64 readBackOffset = stagingBuffer.usedUpSize;

		ToHostCopyFuture future{
			std::move(stagingBuffer),
			timeline,
			readyValue,
			/* readback offset: */ readBackOffset,
		};

		singleCopyBufferToBuffer({
			.src = info.src,
			.dst = *stagingBuffer.buffer,
			.region = {
				.srcOffset = info.region.srcOffset,
				.dstOffset = 0,
				.size = info.region.size,
			}
		});

		return std::move(future);
	}

	//thread_local std::vector<

	void CommandListBackend::multiCopyHostToImage(MultiHostToImageCopyInfo const& ci) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(!currentRenderPass.has_value(), "can not record copy commands in a render pass");
		if (bBarriersQueued) { insertQueuedBarriers(); }

		for (size_t i = 0; i < ci.regions.size(); i++) {
			auto const& region = ci.regions[i];
			auto mappedmem = this->mapMemoryStagedImage(ci.dst,region.subRessource, region.imageOffset, region.imageExtent);
			std::memcpy(mappedmem.hostPtr, ci.src + region.srcOffset, mappedmem.size);
		}
	}
	void CommandListBackend::singleCopyHostToImage(SingleHostToImageCopyInfo const& ci) {
		this->multiCopyHostToImage({ .src = ci.src, .dst = ci.dst, .dstLayout = ci.dstLayout, .regions = std::span{&ci.region,1}});
	}

	void CommandListBackend::multiCopyBufferToBuffer(MultiBufferToBufferCopyInfo const& ci) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(!currentRenderPass.has_value(), "can not record copy commands in a render pass");
		if (bBarriersQueued) { insertQueuedBarriers(); }

#if defined(_DEBUG)
		for (size_t i = 0; i < ci.regions.size(); i++) {
			auto const& region = ci.regions.data()[i];
			DAXA_ASSERT_M(region.srcOffset + region.size <= ci.src.getSize(), "copy overruns buffer");
			DAXA_ASSERT_M(region.dstOffset + region.size <= ci.dst.getSize(), "copy overruns buffer");
		}
#endif

		vkCmdCopyBuffer(cmd, (VkBuffer)ci.src.getVkBuffer(), (VkBuffer)ci.dst.getVkBuffer(), static_cast<u32>(ci.regions.size()), reinterpret_cast<VkBufferCopy const*>(ci.regions.data()));
	}
	void CommandListBackend::singleCopyBufferToBuffer(SingleBufferToBufferCopyInfo const& ci) {
		this->multiCopyBufferToBuffer({ .src = ci.src, .dst = ci.dst, .regions = std::span{&ci.region,1}});
	}

	thread_local std::vector<VkBufferImageCopy> bufferToImageCopyBuffer = {};
	void CommandListBackend::multiCopyBufferToImage(MultiBufferToImageCopyInfo const& ci) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(!currentRenderPass.has_value(), "can not record copy commands in a render pass");
		if (bBarriersQueued) { insertQueuedBarriers(); }

		bufferToImageCopyBuffer.reserve(ci.regions.size());

		for (auto& info: ci.regions) {
			VkBufferImageCopy outinfo = *reinterpret_cast<VkBufferImageCopy const*>(&info);
			if (outinfo.imageExtent.width == 0) {
				outinfo.imageExtent.width = ci.dst->getVkExtent3D().width;
			}
			if (outinfo.imageExtent.height == 0) {
				outinfo.imageExtent.height = ci.dst->getVkExtent3D().height;
			}
			if (outinfo.imageExtent.depth == 0) {
				outinfo.imageExtent.depth = ci.dst->getVkExtent3D().depth;
			}
			auto minNeededBufferSize = byteSizeOfImageRange(ci.dst, outinfo.imageSubresource, outinfo.imageOffset, outinfo.imageExtent);
			DAXA_ASSERT_M(minNeededBufferSize <= ci.src.getSize(), "copy size overruns src size");
			bufferToImageCopyBuffer.push_back(outinfo);
		}

		vkCmdCopyBufferToImage(cmd, (VkBuffer)ci.src.getVkBuffer(), ci.dst->getVkImage(), ci.dstLayout, bufferToImageCopyBuffer.size(), bufferToImageCopyBuffer.data());
		bufferToImageCopyBuffer.clear();
	}
	void CommandListBackend::singleCopyBufferToImage(SingleBufferToImageCopyInfo const& ci) {
		this->multiCopyBufferToImage({ .src = ci.src, .dst = ci.dst, .dstLayout = ci.dstLayout, .regions = std::span{&ci.region,1} });
	}

	thread_local std::vector<VkImageCopy> imagetoImageCopyBuffer = {};
	void CommandListBackend::multiCopyImageToImage(MultiImageToImageCopyInfo const& ci) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(!currentRenderPass.has_value(), "can not record copy commands in a render pass");
		if (bBarriersQueued) { insertQueuedBarriers(); }

		imagetoImageCopyBuffer.reserve(ci.regions.size());
		for (auto& info : ci.regions) {
			VkImageCopy infoout;
			infoout.srcOffset = info.srcOffset;
			infoout.srcSubresource = info.srcSubresource;
			infoout.dstOffset = info.dstOffset;
			infoout.dstSubresource = info.dstSubresource;
			infoout.extent = info.extent;
			if (infoout.extent.width == 0) {
				infoout.extent.width = ci.src->getVkExtent3D().width;
			}
			if (infoout.extent.height == 0) {
				infoout.extent.height = ci.src->getVkExtent3D().height;
			}
			if (infoout.extent.depth == 0) {
				infoout.extent.depth = ci.src->getVkExtent3D().depth;
			}
			imagetoImageCopyBuffer.push_back(infoout);
		}

		vkCmdCopyImage(cmd, ci.src->getVkImage(), ci.srcLayout, ci.dst->getVkImage(), ci.dstLayout, imagetoImageCopyBuffer.size(), imagetoImageCopyBuffer.data());
		imagetoImageCopyBuffer.clear();
	}
	void CommandListBackend::singleCopyImageToImage(SingleImageToImageCopyInfo const& ci) {
		this->multiCopyImageToImage({ .src = ci.src, .srcLayout = ci.srcLayout, .dst = ci.dst, .dstLayout = ci.dstLayout, .regions = std::span{&ci.region,1}});
	}


	///
	/// 	TRANSFER 2 END
	///



	void CommandListBackend::pushConstant(VkShaderStageFlags shaderStage, void const* daxa, u32 size, u32 offset) {
		vkCmdPushConstants(cmd, (**currentPipeline).getVkPipelineLayout(), shaderStage, offset, size, daxa);
	}

	void CommandListBackend::dispatch(u32 groupCountX, u32 groupCountY, u32 grpupCountZ) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M((**currentPipeline).getVkBindPoint() == VK_PIPELINE_BIND_POINT_COMPUTE, "can not dispatch compute commands with out a bound compute pipeline.");
		if (bBarriersQueued) { insertQueuedBarriers(); }
		vkCmdDispatch(cmd, groupCountX, groupCountY, grpupCountZ);
	}

	void CommandListBackend::bindVertexBuffer(u32 binding, BufferHandle& buffer, size_t bufferOffset) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(buffer, "invalid buffer handle");
		auto vkBuffer = (VkBuffer)buffer.getVkBuffer();
		vkCmdBindVertexBuffers(cmd, binding, 1, &vkBuffer, &bufferOffset);
	}

	void CommandListBackend::bindIndexBuffer(BufferHandle& buffer, size_t bufferOffset, VkIndexType indexType) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(buffer, "invalid buffer handle");
		auto vkBuffer = (VkBuffer)buffer.getVkBuffer();
		vkCmdBindIndexBuffer(cmd, vkBuffer, bufferOffset, indexType);
	}

	void CommandListBackend::beginRendering(BeginRenderingInfo ri) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(!currentRenderPass.has_value(), "can not begin renderpass while recording a render pass");
		if (bBarriersQueued) { insertQueuedBarriers(); }
		std::vector<RenderAttachmentInfo> colorAttachments;
		colorAttachments.reserve(ri.colorAttachments.size());
		for (auto& a : ri.colorAttachments) {
			colorAttachments.push_back(a);
		}
		currentRenderPass = CurrentRenderPass{
			.colorAttachments = std::move(colorAttachments),
			.depthAttachment = ri.depthAttachment == nullptr ? std::optional<RenderAttachmentInfo>{} : *ri.depthAttachment,
			.stencilAttachment = ri.stencilAttachment == nullptr ? std::optional<RenderAttachmentInfo>{} : *ri.stencilAttachment,
		};
		operationsInProgress += 1;
		for (int i = 0; i < ri.colorAttachments.size(); i++) {
			renderAttachmentBuffer.push_back(VkRenderingAttachmentInfoKHR{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
				.pNext = nullptr,
				.imageView = ri.colorAttachments[i].image->getVkImageView(),
				.imageLayout = ri.colorAttachments[i].layout,
				.resolveMode = VK_RESOLVE_MODE_NONE,// ri.colorAttachments[i].resolveMode,
				.loadOp = ri.colorAttachments[i].loadOp,
				.storeOp = ri.colorAttachments[i].storeOp,
				.clearValue = ri.colorAttachments[i].clearValue
			});
		}

		std::optional<VkRenderingAttachmentInfoKHR> depthAttachmentInfo = ri.depthAttachment == nullptr ? std::nullopt :
			std::optional{
				VkRenderingAttachmentInfoKHR{
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
					.pNext = nullptr,
					.imageView = ri.depthAttachment->image->getVkImageView(),
					.imageLayout = ri.depthAttachment->layout,
					.resolveMode = ri.depthAttachment->resolveMode,
					.loadOp = ri.depthAttachment->loadOp,
					.storeOp = ri.depthAttachment->storeOp,
					.clearValue = ri.depthAttachment->clearValue,
				}
			};

		std::optional<VkRenderingAttachmentInfoKHR> stencilAttachmentInfo = ri.stencilAttachment == nullptr ? std::nullopt :
			std::optional{
				VkRenderingAttachmentInfoKHR{
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
					.pNext = nullptr,
					.imageView = ri.stencilAttachment->image->getVkImageView(),
					.imageLayout = ri.stencilAttachment->layout,
					.resolveMode = ri.stencilAttachment->resolveMode,
					.loadOp = ri.stencilAttachment->loadOp,
					.storeOp = ri.stencilAttachment->storeOp,
					.clearValue = ri.stencilAttachment->clearValue,
				}
			};

		VkRenderingInfoKHR renderInfo{};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderInfo.pNext = nullptr;
		if (ri.renderArea) {
			renderInfo.renderArea = *ri.renderArea;
		}
		else if (ri.colorAttachments.size() > 0) {
			renderInfo.renderArea.extent.width = ri.colorAttachments[0].image->getImageHandle()->getVkExtent3D().width;
			renderInfo.renderArea.extent.height = ri.colorAttachments[0].image->getImageHandle()->getVkExtent3D().height;
		}
		else if (ri.depthAttachment != nullptr) {
			renderInfo.renderArea.extent.width = ri.depthAttachment->image->getImageHandle()->getVkExtent3D().width;
			renderInfo.renderArea.extent.height = ri.depthAttachment->image->getImageHandle()->getVkExtent3D().height;
		}
		else if (ri.stencilAttachment != nullptr) {
			renderInfo.renderArea.extent.width = ri.stencilAttachment->image->getImageHandle()->getVkExtent3D().width;
			renderInfo.renderArea.extent.height = ri.stencilAttachment->image->getImageHandle()->getVkExtent3D().height;
		}	// otherwise let it be zero, as we dont render anything anyways

		renderInfo.layerCount = 1;	// Not sure what this does

		renderInfo.colorAttachmentCount = ri.colorAttachments.size();
		renderInfo.pColorAttachments = renderAttachmentBuffer.data();
		renderInfo.pDepthAttachment = depthAttachmentInfo.has_value() ? &depthAttachmentInfo.value() : nullptr;
		renderInfo.pStencilAttachment = stencilAttachmentInfo.has_value() ? &stencilAttachmentInfo.value() : nullptr;

		deviceBackend->vkCmdBeginRenderingKHR(cmd, (VkRenderingInfoKHR*)&renderInfo);

		renderAttachmentBuffer.clear();

		setViewport(VkViewport{
			.x = 0,
			.y = 0,
			.width = (f32)renderInfo.renderArea.extent.width,
			.height = (f32)renderInfo.renderArea.extent.height,
			.minDepth = 0,
			.maxDepth = 1,
		});

		checkIfPipelineAndRenderPassFit();
	}
	void CommandListBackend::endRendering() {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(currentRenderPass.has_value(), "can only end render pass when there is one in progress");
		currentRenderPass = {};
		operationsInProgress -= 1;
		deviceBackend->vkCmdEndRenderingKHR(cmd);
	}

	void CommandListBackend::bindPipeline(PipelineHandle& pipeline) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(pipeline, "invalid pipeline handle");
		vkCmdBindPipeline(cmd, pipeline->getVkBindPoint(), pipeline->getVkPipeline());
		usedGraphicsPipelines.push_back(pipeline);

		currentPipeline = pipeline;

		checkIfPipelineAndRenderPassFit();
	}

	void CommandListBackend::unbindPipeline() {
		DAXA_ASSERT_M(currentPipeline, "can only unbind pipeline, when there is a pipeline bound");
		currentPipeline = {};
	}

	void CommandListBackend::begin() {
		VkCommandBufferBeginInfo cbbi{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		};
		DAXA_CHECK_VK_RESULT(vkBeginCommandBuffer(cmd, &cbbi));
	}

	void CommandListBackend::checkIfPipelineAndRenderPassFit() {
		if (currentRenderPass.has_value() && currentPipeline) {
			std::string nameMessage = "; pipeline name: ";
			nameMessage += (**currentPipeline).getDebugName();

			auto& currentPipeCAFs = (**currentPipeline).getColorAttachemntFormats();
			auto& currentRendPassCAIs = currentRenderPass.value().colorAttachments;

			DAXA_ASSERT_M(currentRendPassCAIs.size() >= currentPipeCAFs.size(), std::string("renderpass must have at least as many attachments as the bound pipeline") + nameMessage);

			for (size_t i = 0; i < currentPipeCAFs.size(); i++) {
				auto pipeformat = currentPipeCAFs[i];
				auto rpformat = currentRendPassCAIs[i].image->getVkFormat();
				std::string formatMessage = "; pipeline attachment format [";
				formatMessage += std::to_string(i);
				formatMessage += "]: " ;
				formatMessage += std::to_string((int)pipeformat);
				formatMessage += "; renderpass attachment format [";
				formatMessage += std::to_string(i);
				formatMessage += "]: " ;
				formatMessage += std::to_string((int)rpformat);
				DAXA_ASSERT_M(pipeformat == rpformat, std::string("renderpass color attachment formats must match the ones off the bound pipeline") + nameMessage + formatMessage);
			}
			if ((**currentPipeline).getDepthAttachmentFormat() != VK_FORMAT_UNDEFINED) {
				DAXA_ASSERT_M(currentRenderPass->depthAttachment.has_value(), std::string("if the pipeline uses a depth attachment the renderpass must have a depth attachemnt too") + nameMessage);
				DAXA_ASSERT_M(currentRenderPass->depthAttachment.value().image->getVkFormat() == (**currentPipeline).getDepthAttachmentFormat(), std::string("the depth attachment format of the pipeline and renderpass must be the same") + nameMessage);
			}
			if ((**currentPipeline).getStencilAttachmentFormat() != VK_FORMAT_UNDEFINED) {
				DAXA_ASSERT_M(currentRenderPass->stencilAttachment.has_value(), std::string("if the pipeline uses a stencil attachment the renderpass must have a stencil attachemnt too") + nameMessage);
				DAXA_ASSERT_M(currentRenderPass->stencilAttachment.value().image->getVkFormat() == (**currentPipeline).getStencilAttachmentFormat(), std::string("the stencil attachment format of the pipeline and renderpass must be the same") + nameMessage);
			}
		}
	}

	void CommandListBackend::setDebugName(char const* debugName) {
		if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && debugName != nullptr) {
			this->debugName = debugName;
			VkDebugUtilsObjectNameInfoEXT nameInfo {
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER,
				.objectHandle = (uint64_t)this->cmd,
				.pObjectName = debugName,
			};
			instance->pfnSetDebugUtilsObjectNameEXT(deviceBackend->device.device, &nameInfo);
			nameInfo = VkDebugUtilsObjectNameInfoEXT {
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_COMMAND_POOL,
				.objectHandle = (uint64_t)this->cmdPool,
				.pObjectName = debugName,
			};
			instance->pfnSetDebugUtilsObjectNameEXT(deviceBackend->device.device, &nameInfo);
		}
	}

	void CommandListBackend::reset() {
		DAXA_ASSERT_M(finalized || empty, "can not reset non empty command list that is not finalized");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(operationsInProgress == 0, "can not reset command list with recordings in progress");
		empty = true;
		DAXA_CHECK_VK_RESULT(vkResetCommandPool(deviceBackend->device.device, cmdPool, VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));
		usedGraphicsPipelines.clear();
		usedSets.clear();
		currentPipeline = {};
		currentRenderPass = {};
		usedUploadStagingBuffers.clear();
		usedDownloadStagingBuffers.clear();
		usedTimelines.clear();
		finalized = false;
		{
			// remove zombie list from active zombie lists.
			// this prevents any handle to queue their zombie into this list.
			// this is important as an image view can own a sampler,
			// if the sampler becomes a zombie while the zombie queue is beein cleared, the zombie will be enqueued in all active queues,
			// including this one. This causes a memory corruption as its illegal to append to a vector that is currently running its clear function.
			std::unique_lock lock(deviceBackend->graveyard.mtx);
			auto iter = std::find_if(deviceBackend->graveyard.activeZombieLists.begin(), deviceBackend->graveyard.activeZombieLists.end(), [&](std::shared_ptr<ZombieList>& other){ return other.get() == zombies.get(); });
			if (iter != deviceBackend->graveyard.activeZombieLists.end()) {
				deviceBackend->graveyard.activeZombieLists.erase(iter);
			}
		}
		// zombie list is cleared after it is removed from the pool of active zombie lists.
		// this prevents image views to enqueue sampler zombies while the queue is cleared.
		zombies->zombies.clear();
	}

	void CommandListBackend::setViewport(VkViewport const& viewport) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(currentRenderPass.has_value(), "can only set viewports inside a renderpass");
		vkCmdSetViewport(cmd, 0, 1, &viewport); 
		VkRect2D scissor{
			.offset = { 0, 0 },
			.extent = { (u32)viewport.width, (u32)viewport.height },
		};
		setScissor(scissor);
	}

	void CommandListBackend::setScissor(VkRect2D const& scissor) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(currentRenderPass.has_value(), "can only set scissor inside a renderpass");
		vkCmdSetScissor(cmd, 0, 1, &scissor);
	}
	
	void CommandListBackend::draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(currentPipeline, "can not draw sets if there is no pipeline bound");
		vkCmdDraw(cmd, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandListBackend::drawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstIntance) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(currentPipeline, "can not draw sets if there is no pipeline bound");
		vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstIntance);
	}

	void CommandListBackend::bindSet(u32 setBinding, BindingSetHandle set) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(currentPipeline, "can not bind sets if there is no pipeline bound");
		vkCmdBindDescriptorSets(cmd, (**currentPipeline).getVkBindPoint(), (**currentPipeline).getVkPipelineLayout(), setBinding, 1, &set->set, 0, nullptr);
		usedSets.push_back(std::move(set));
	}

	void CommandListBackend::bindSetPipelineIndependant(u32 setBinding, BindingSetHandle set, VkPipelineBindPoint bindPoint, VkPipelineLayout layout) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		vkCmdBindDescriptorSets(cmd, bindPoint, layout, setBinding, 1, &set->set, 0, nullptr);
		usedSets.push_back(std::move(set));
	}

	void CommandListBackend::bindAll(u32 set) {
		vkCmdBindDescriptorSets(cmd, (**currentPipeline).getVkBindPoint(), (**currentPipeline).getVkPipelineLayout(), set, 1, &deviceBackend->bindAllSet, 0, nullptr);
	}

	void CommandListBackend::insertQueuedBarriers() {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(!currentRenderPass.has_value(), "can not insert memory barriers in renderpass");

		if (bBarriersQueued) {
			VkDependencyInfoKHR dependencyInfo{
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR,
				.pNext = nullptr,
				.memoryBarrierCount = (u32)queuedMemoryBarriers.size(),
				.pMemoryBarriers = queuedMemoryBarriers.data(),
				.bufferMemoryBarrierCount = 0,
				.pBufferMemoryBarriers = nullptr,
				.imageMemoryBarrierCount = (u32)queuedImageBarriers.size(),
				.pImageMemoryBarriers = queuedImageBarriers.data(),
			};

			queuedMemoryBarriers.clear();
			queuedBufferBarriers.clear();
			queuedImageBarriers.clear();

			deviceBackend->vkCmdPipelineBarrier2KHR(cmd, &dependencyInfo);
			this->bBarriersQueued = false;
		}
	}

	void CommandListBackend::queueMemoryBarrier(MemoryBarrier const& barrier) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(!currentRenderPass.has_value(), "can not insert memory barriers in renderpass");
		queuedMemoryBarriers.push_back(VkMemoryBarrier2KHR{
			.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR,
			.pNext = nullptr,
			.srcStageMask = barrier.srcStages,
			.srcAccessMask = barrier.srcAccess,
			.dstStageMask = barrier.dstStages,
			.dstAccessMask = barrier.dstAccess,
		});
		bBarriersQueued = true;
	}

	void CommandListBackend::queueImageBarrier(ImageBarrier const& imgBarrier) {
		DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
		DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
		DAXA_ASSERT_M(!currentRenderPass.has_value(), "can not insert memory barriers in renderpass");

		VkImageSubresourceRange range = imgBarrier.subRange;
		if (range.aspectMask == VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM) {
			range = imgBarrier.image->getVkImageSubresourceRange();
		}

		queuedImageBarriers.push_back(VkImageMemoryBarrier2KHR{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR,
			.pNext = nullptr,
			.srcStageMask = imgBarrier.barrier.srcStages,
			.srcAccessMask = imgBarrier.barrier.srcAccess,
			.dstStageMask = imgBarrier.barrier.dstStages,
			.dstAccessMask = imgBarrier.barrier.dstAccess,
			.oldLayout = imgBarrier.layoutBefore,
			.newLayout = imgBarrier.layoutAfter,
			.srcQueueFamilyIndex = imgBarrier.srcQueueIndex,
			.dstQueueFamilyIndex = imgBarrier.dstQueueIndex,
			.image = imgBarrier.image->getImageHandle()->getVkImage(),
			.subresourceRange = range,
		});
		bBarriersQueued = true;
	}

	VkCommandBuffer CommandListBackend::getVkCommandBuffer() { return cmd; }

	VkCommandPool CommandListBackend::getVkCommandPool() { return cmdPool; }

	std::string const& CommandListBackend::getDebugName() const { return debugName; }
}