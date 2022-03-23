#include "CommandList.hpp"

#include "Instance.hpp"
#include "util.hpp"

namespace daxa {
	namespace gpu {
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

		CommandList::CommandList() {
			this->renderAttachmentBuffer.reserve(5);
			usedSets.reserve(10);
			usedGraphicsPipelines.reserve(3);
			usedStagingBuffers.reserve(1);
		}

		CommandList::~CommandList() {
			if (deviceBackend) {
				DAXA_ASSERT_M(operationsInProgress == 0, "a command list can not be descroyed when there are still commands recorded");
				DAXA_ASSERT_M(empty, "a command list can not be destroyed when not empty");
				vkFreeCommandBuffers(deviceBackend->device.device, cmdPool, 1, &cmd);
				vkDestroyCommandPool(deviceBackend->device.device, cmdPool, nullptr);
				deviceBackend = VK_NULL_HANDLE;
			}
		}

		void CommandList::finalize() {
			if (bBarriersQueued) { insertQueuedBarriers(); }
			DAXA_ASSERT_M(finalized == false, "can not finalize a command list twice");
			DAXA_ASSERT_M(operationsInProgress == 0, "can only finalize a command list that has no operations in progress");
			DAXA_CHECK_VK_RESULT(vkEndCommandBuffer(cmd));
			finalized = true;
		}

		MappedMemoryPointer<u8> CommandList::mapMemoryStagedBufferVoid(BufferHandle copyDst, size_t size, size_t dstOffset) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(size <= STAGING_BUFFER_POOL_BUFFER_SIZE, "Currently uploads over a size of 67.108.864 bytes are not supported by the uploadToBuffer function. Please use a staging buffer.");
			if (bBarriersQueued) { insertQueuedBarriers(); }
			
			if (usedStagingBuffers.empty() || usedStagingBuffers.back().getLeftOverSize() < size) {
				usedStagingBuffers.push_back(stagingBufferPool.lock()->getStagingBuffer());
			}

			auto& stagingBuffer = usedStagingBuffers.back();

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

		MappedMemoryPointer<u8> CommandList::mapMemoryStagedImageVoid(ImageHandle copyDst, VkImageSubresourceLayers subRessource, VkOffset3D dstOffset, VkExtent3D dstExtent) {
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
			
			if (usedStagingBuffers.empty() || usedStagingBuffers.back().getLeftOverSize() < size) {
				usedStagingBuffers.push_back(stagingBufferPool.lock()->getStagingBuffer());
			}

			auto& stagingBuffer = usedStagingBuffers.back();
			
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

		void CommandList::copyHostToBuffer(HostToBufferCopyInfo copyInfo) {
			singleCopyHostToBuffer({
				.src = reinterpret_cast<u8*>(copyInfo.src),
				.dst = copyInfo.dst,
				.region = {
					.srcOffset = 0,
					.dstOffset = copyInfo.dstOffset,
					.size = copyInfo.size,
				}
			});
		}

		void CommandList::copyHostToImage(HostToImageCopyInfo copyInfo) {
			HostToImageCopyRegion forwardCopyInfo = {};
			if (copyInfo.dstImgSubressource.has_value()) {
				forwardCopyInfo.subRessource = *copyInfo.dstImgSubressource;
			}
			singleCopyHostToImage({
				.src = reinterpret_cast<u8*>(copyInfo.src),
				.dst = copyInfo.dst->getImageHandle(),
				.dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.region = forwardCopyInfo,
			});
		}

		void CommandList::copyHostToImageSynced(HostToImageCopySyncedInfo copySyncedInfo) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			if (bBarriersQueued) { insertQueuedBarriers(); }
			ImageBarrier firstBarrier{
				.barrier = FULL_MEMORY_BARRIER,
				.image = copySyncedInfo.dst,
				.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
				.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			};
			insertBarriers({},{&firstBarrier, 1});

			copyHostToImage({
				.src = copySyncedInfo.src,
				.dst = copySyncedInfo.dst, 
				.dstImgSubressource = copySyncedInfo.dstImgSubressource,
				.size = copySyncedInfo.size,
			});

			ImageBarrier secondBarrier{
				.barrier = FULL_MEMORY_BARRIER,
				.image = copySyncedInfo.dst,
				.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.layoutAfter = copySyncedInfo.dstFinalLayout,
			};
			insertBarriers({},{&secondBarrier, 1});
		}

		void CommandList::copyMultiBufferToBuffer(BufferToBufferMultiCopyInfo copyInfo) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu.");
			DAXA_ASSERT_M(copyInfo.regions.size() > 0, "amount of copy regions must be greater than 0.");
			if (bBarriersQueued) { insertQueuedBarriers(); }
			for (int i = 0; i < copyInfo.regions.size(); i++) {
				DAXA_ASSERT_M(copyInfo.src->getSize() >= copyInfo.regions[i].size + copyInfo.regions[i].srcOffset, "ERROR: src buffer is smaller than the region that shouly be copied!");
				DAXA_ASSERT_M(copyInfo.dst->getSize() >= copyInfo.regions[i].size + copyInfo.regions[i].dstOffset, "ERROR: dst buffer is smaller than the region that shouly be copied!");
			}
			vkCmdCopyBuffer(cmd, copyInfo.src->getVkBuffer(), copyInfo.dst->getVkBuffer(), copyInfo.regions.size(), (VkBufferCopy*)copyInfo.regions.data());	// THIS COULD BREAK ON ABI CHANGE
		}

		void CommandList::copyBufferToBuffer(BufferToBufferCopyInfo copyInfo) {
			singleCopyBufferToBuffer({
				.src = copyInfo.src,
				.dst = copyInfo.dst,
				.region = {
					.srcOffset = copyInfo.region.srcOffset,
					.dstOffset = copyInfo.region.dstOffset,
					.size = copyInfo.region.size,
				},
			});
		}

		void CommandList::copyBufferToImage(BufferToImageCopyInfo copyInfo) {
			BufferToImageCopyRegion forwardCopyInfo = {};
			forwardCopyInfo.bufferOffset = copyInfo.srcOffset;
			if (copyInfo.subRessourceLayers.has_value()) {
				forwardCopyInfo.subRessource = copyInfo.subRessourceLayers.value();
			}
			singleCopyBufferToImage({
				.src = copyInfo.src,
				.dst = copyInfo.dst->getImageHandle(),
				.dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.region = forwardCopyInfo,
			});
		}

		void CommandList::copyImageToImage(ImageToImageCopyInfo copyInfo) {

			ImageToImageCopyRegion forwardCopyInfo = {};
			forwardCopyInfo.srcOffset = copyInfo.srcOffset;
			forwardCopyInfo.dstOffset = copyInfo.dstOffset;
			forwardCopyInfo.extent = copyInfo.size;
			singleCopyImageToImage({
				.src = copyInfo.src->getImageHandle(),
				.srcLayout = copyInfo.srcLayout,
				.dst = copyInfo.dst->getImageHandle(),
				.dstLayout = copyInfo.dstLayout,
				.region = forwardCopyInfo,
			});
		}

		void CommandList::copyImageToImageSynced(ImageToImageCopySyncedInfo copySyncedInfo) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			insertImageBarriers(std::array{
				ImageBarrier{ 
					.barrier = FULL_MEMORY_BARRIER,
					.image = copySyncedInfo.src, 
					.layoutBefore = copySyncedInfo.srcLayoutBeforeAndAfter , 
					.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL 
				},
				ImageBarrier{ 
					.barrier = FULL_MEMORY_BARRIER,
					.image = copySyncedInfo.dst, 
					.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED, 
					.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL 
				},
			});
			copyImageToImage({ 
				.src = copySyncedInfo.src, 
				.srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.srcOffset = copySyncedInfo.srcOffset,
				.dst = copySyncedInfo.dst,
				.dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.dstOffset = copySyncedInfo.dstOffset,
				.size = copySyncedInfo.size,
			});
			insertImageBarriers(std::array{
				ImageBarrier{ 
					.barrier = FULL_MEMORY_BARRIER,
					.image = std::move(copySyncedInfo.src), 
					.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
					.layoutAfter = copySyncedInfo.srcLayoutBeforeAndAfter 
				},
				ImageBarrier{ 
					.barrier = FULL_MEMORY_BARRIER,
					.image = std::move(copySyncedInfo.dst), 
					.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
					.layoutAfter = copySyncedInfo.dstFinalLayout
				},
			});
		}


		///
		/// 	TRANSFER 2 BEGIN
		///


		void CommandList::multiCopyHostToBuffer(MultiHostToBufferCopyInfo const& ci) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(!currentRenderPass.has_value(), "can not record copy commands in a render pass");
			if (bBarriersQueued) { insertQueuedBarriers(); }

			for (size_t i = 0; i < ci.regions.size(); i++) {
				auto const& info = ci.regions[i];
				auto mappedmem = this->mapMemoryStagedBuffer<u8 const*>(ci.dst, info.size, info.dstOffset);
				std::memcpy(mappedmem.hostPtr, ci.src + info.srcOffset, info.size);
			}
		}
		void CommandList::singleCopyHostToBuffer(SingleCopyHostToBufferInfo const& ci) {
			this->multiCopyHostToBuffer({ .src = ci.src, .dst = ci.dst, .regions = std::span{&ci.region, 1} });
		}

		void CommandList::multiCopyHostToImage(MultiHostToImageCopyInfo const& ci) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(!currentRenderPass.has_value(), "can not record copy commands in a render pass");
			if (bBarriersQueued) { insertQueuedBarriers(); }

			for (size_t i = 0; i < ci.regions.size(); i++) {
				auto const& region = ci.regions[i];
				auto mappedmem = this->mapMemoryStagedImage<u8 const*>(ci.dst,region.subRessource, region.imageOffset, region.imageExtent);
				std::memcpy(mappedmem.hostPtr, ci.src + region.srcOffset, mappedmem.size);
			}
		}
		void CommandList::singleCopyHostToImage(SingleHostToImageCopyInfo const& ci) {
			this->multiCopyHostToImage({ .src = ci.src, .dst = ci.dst, .dstLayout = ci.dstLayout, .regions = std::span{&ci.region,1}});
		}

		void CommandList::multiCopyBufferToBuffer(MultiBufferToBufferCopyInfo const& ci) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(!currentRenderPass.has_value(), "can not record copy commands in a render pass");
			if (bBarriersQueued) { insertQueuedBarriers(); }

#if defined(_DEBUG)
			for (size_t i = 0; i < ci.regions.size(); i++) {
				auto const& region = ci.regions.data()[i];
				DAXA_ASSERT_M(region.srcOffset + region.size <= ci.src->getSize(), "copy overruns buffer");
				DAXA_ASSERT_M(region.dstOffset + region.size <= ci.dst->getSize(), "copy overruns buffer");
			}
#endif

			vkCmdCopyBuffer(cmd, ci.src->getVkBuffer(), ci.dst->getVkBuffer(), static_cast<u32>(ci.regions.size()), reinterpret_cast<VkBufferCopy const*>(ci.regions.data()));
		}
		void CommandList::singleCopyBufferToBuffer(SingleBufferToBufferCopyInfo const& ci) {
			this->multiCopyBufferToBuffer({ .src = ci.src, .dst = ci.dst, .regions = std::span{&ci.region,1}});
		}

		thread_local std::vector<VkBufferImageCopy> bufferToImageCopyBuffer = {};
		void CommandList::multiCopyBufferToImage(MultiBufferToImageCopyInfo const& ci) {
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
				DAXA_ASSERT_M(minNeededBufferSize <= ci.src->getSize(), "copy size overruns src size");
				bufferToImageCopyBuffer.push_back(outinfo);
			}

			vkCmdCopyBufferToImage(cmd, ci.src->getVkBuffer(), ci.dst->getVkImage(), ci.dstLayout, bufferToImageCopyBuffer.size(), bufferToImageCopyBuffer.data());
			bufferToImageCopyBuffer.clear();
		}
		void CommandList::singleCopyBufferToImage(SingleBufferToImageCopyInfo const& ci) {
			this->multiCopyBufferToImage({ .src = ci.src, .dst = ci.dst, .dstLayout = ci.dstLayout, .regions = std::span{&ci.region,1} });
		}

		thread_local std::vector<VkImageCopy> imagetoImageCopyBuffer = {};
		void CommandList::multiCopyImageToImage(MultiImageToImageCopyInfo const& ci) {
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
		void CommandList::singleCopyImageToImage(SingleImageToImageCopyInfo const& ci) {
			this->multiCopyImageToImage({ .src = ci.src, .srcLayout = ci.srcLayout, .dst = ci.dst, .dstLayout = ci.dstLayout, .regions = std::span{&ci.region,1}});
		}


		///
		/// 	TRANSFER 2 END
		///


		void CommandList::dispatch(u32 groupCountX, u32 groupCountY, u32 grpupCountZ) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M((**currentPipeline).getVkBindPoint() == VK_PIPELINE_BIND_POINT_COMPUTE, "can not dispatch compute commands with out a bound compute pipeline.");
			if (bBarriersQueued) { insertQueuedBarriers(); }
			vkCmdDispatch(cmd, groupCountX, groupCountY, grpupCountZ);
		}

		void CommandList::bindVertexBuffer(u32 binding, BufferHandle& buffer, size_t bufferOffset) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(buffer, "invalid buffer handle");
			auto vkBuffer = buffer->getVkBuffer();
			vkCmdBindVertexBuffers(cmd, binding, 1, &vkBuffer, &bufferOffset);
		}

		void CommandList::bindIndexBuffer(BufferHandle& buffer, size_t bufferOffset, VkIndexType indexType) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(buffer, "invalid buffer handle");
			auto vkBuffer = buffer->getVkBuffer();
			vkCmdBindIndexBuffer(cmd, vkBuffer, bufferOffset, indexType);
		}

		void CommandList::beginRendering(BeginRenderingInfo ri) {
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
				.depthAttachment = ri.depthAttachment == nullptr ? std::optional<gpu::RenderAttachmentInfo>{} : *ri.depthAttachment,
				.stencilAttachment = ri.stencilAttachment == nullptr ? std::optional<gpu::RenderAttachmentInfo>{} : *ri.stencilAttachment,
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
		void CommandList::endRendering() {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(currentRenderPass.has_value(), "can only end render pass when there is one in progress");
			currentRenderPass = {};
			operationsInProgress -= 1;
			deviceBackend->vkCmdEndRenderingKHR(cmd);
		}

		void CommandList::bindPipeline(PipelineHandle& pipeline) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(pipeline, "invalid pipeline handle");
			vkCmdBindPipeline(cmd, pipeline->getVkBindPoint(), pipeline->getVkPipeline());
			usedGraphicsPipelines.push_back(pipeline);

			currentPipeline = pipeline;

			checkIfPipelineAndRenderPassFit();
		}

		void CommandList::unbindPipeline() {
			DAXA_ASSERT_M(currentPipeline, "can only unbind pipeline, when there is a pipeline bound");
			currentPipeline = {};
		}

		void CommandList::begin() {
			VkCommandBufferBeginInfo cbbi{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			};
			DAXA_CHECK_VK_RESULT(vkBeginCommandBuffer(cmd, &cbbi));
		}

		void CommandList::checkIfPipelineAndRenderPassFit() {
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

		void CommandList::setDebugName(char const* debugName) {
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

		void CommandList::reset() {
			DAXA_ASSERT_M(finalized || empty, "can not reset non empty command list that is not finalized");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(operationsInProgress == 0, "can not reset command list with recordings in progress");
			empty = true;
			DAXA_CHECK_VK_RESULT(vkResetCommandPool(deviceBackend->device.device, cmdPool, VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));
			usedGraphicsPipelines.clear();
			usedSets.clear();
			currentPipeline = {};
			currentRenderPass = {};
			usedStagingBuffers.clear();
			finalized = false;
			zombies->zombies.clear();
			{
				std::unique_lock lock(deviceBackend->graveyard.mtx);
				auto iter = std::find_if(deviceBackend->graveyard.activeZombieLists.begin(), deviceBackend->graveyard.activeZombieLists.end(), [&](std::shared_ptr<ZombieList>& other){ return other.get() == zombies.get(); });
				if (iter != deviceBackend->graveyard.activeZombieLists.end()) {
					deviceBackend->graveyard.activeZombieLists.erase(iter);
				}
			}
		}

		void CommandList::setViewport(VkViewport const& viewport) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			vkCmdSetViewport(cmd, 0, 1, &viewport); 
			VkRect2D scissor{
				.offset = { 0, 0 },
				.extent = { (u32)viewport.width, (u32)viewport.height },
			};
			setScissor(scissor);
		}

		void CommandList::setScissor(VkRect2D const& scissor) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			vkCmdSetScissor(cmd, 0, 1, &scissor);
		}
		
		void CommandList::draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(currentPipeline, "can not draw sets if there is no pipeline bound");
			vkCmdDraw(cmd, vertexCount, instanceCount, firstVertex, firstInstance);
		}

		void CommandList::drawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstIntance) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(currentPipeline, "can not draw sets if there is no pipeline bound");
			vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstIntance);
		}

		void CommandList::bindSet(u32 setBinding, BindingSetHandle set) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(currentPipeline, "can not bind sets if there is no pipeline bound");
			vkCmdBindDescriptorSets(cmd, (**currentPipeline).getVkBindPoint(), (**currentPipeline).getVkPipelineLayout(), setBinding, 1, &set->set, 0, nullptr);
			usedSets.push_back(std::move(set));
		}

		void CommandList::bindSetPipelineIndependant(u32 setBinding, BindingSetHandle set, VkPipelineBindPoint bindPoint, VkPipelineLayout layout) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			vkCmdBindDescriptorSets(cmd, bindPoint, layout, setBinding, 1, &set->set, 0, nullptr);
			usedSets.push_back(std::move(set));
		}

		void CommandList::insertBarriers(std::span<MemoryBarrier> memBarriers, std::span<ImageBarrier> imgBarriers) {
			DAXA_ASSERT_M(finalized == false, "can not record any commands to a finished command list");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(!currentRenderPass.has_value(), "can not insert memory barriers in renderpass");
			std::array<VkMemoryBarrier2KHR, 32> memBarrierBuffer;
			u32 memBarrierBufferSize = 0;
			std::array<VkBufferMemoryBarrier2KHR, 32> bufBarrierBuffer;
			u32 bufBarrierBufferSize = 0;
			std::array<VkImageMemoryBarrier2KHR, 32> imgBarrierBuffer;
			u32 imgBarrierBufferSize = 0;

			for (auto& barrier : memBarriers) {
				DAXA_ASSERT_M(memBarrierBufferSize < 32, "can only insert 32 barriers of one kind in a single insertBarriers call");
				memBarrierBuffer[memBarrierBufferSize++] = VkMemoryBarrier2KHR{
					.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR,
					.pNext = nullptr,
					.srcStageMask = barrier.srcStages,
					.srcAccessMask = barrier.srcAccess,
					.dstStageMask = barrier.dstStages,
					.dstAccessMask = barrier.dstAccess,
				};
			}

			for (auto imgBarrier : imgBarriers) {
				DAXA_ASSERT_M(bufBarrierBufferSize < 32, "can only insert 32 barriers of one kind in a single insertBarriers call");

				if (imgBarrier.srcQueueIndex == imgBarrier.dstQueueIndex) {
					imgBarrier.srcQueueIndex = VK_QUEUE_FAMILY_IGNORED;
					imgBarrier.dstQueueIndex = VK_QUEUE_FAMILY_IGNORED;
				}

				VkImageSubresourceRange range = imgBarrier.subRange;
				if (range.aspectMask == VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM) {
					range = imgBarrier.image->getVkImageSubresourceRange();
				}

				imgBarrierBuffer[imgBarrierBufferSize] = VkImageMemoryBarrier2KHR{
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
				};

				imgBarrierBufferSize++;
			}

			VkDependencyInfoKHR dependencyInfo{
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR,
				.pNext = nullptr,
				.memoryBarrierCount = memBarrierBufferSize,
				.pMemoryBarriers = memBarrierBuffer.data(),
				.bufferMemoryBarrierCount = 0,
				.pBufferMemoryBarriers = nullptr,
				.imageMemoryBarrierCount = imgBarrierBufferSize,
				.pImageMemoryBarriers = imgBarrierBuffer.data(),
			};

			deviceBackend->vkCmdPipelineBarrier2KHR(cmd, &dependencyInfo);
		}

		void CommandList::bindAll(u32 set) {
			vkCmdBindDescriptorSets(cmd, (**currentPipeline).getVkBindPoint(), (**currentPipeline).getVkPipelineLayout(), set, 1, &deviceBackend->bindAllSet, 0, nullptr);
		}

		void CommandList::insertQueuedBarriers() {
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

		void CommandList::queueMemoryBarrier(MemoryBarrier const& barrier) {
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

		void CommandList::queueImageBarrier(ImageBarrier const& imgBarrier) {
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
	}
}
