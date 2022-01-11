#include "CommandList.hpp"

namespace daxa {
	namespace gpu {
		size_t roundUpToMultipleOf128(size_t n) {
			size_t remainder = n % 128;
			n -= remainder;
			n += 128 * size_t(bool(remainder));
			return n;
		}

		CommandList::CommandList() {
			this->renderAttachmentBuffer.reserve(5);
			usedImages.reserve(10);
			usedBuffers.reserve(10);
			usedSets.reserve(10);
			usedGraphicsPipelines.reserve(3);
			usedStagingBuffers.reserve(1);
		}

		CommandList::~CommandList() {
			if (device) {
				DAXA_ASSERT_M(operationsInProgress == 0, "a command list can not be descroyed when there are still commands recorded");
				DAXA_ASSERT_M(empty, "a command list can not be destroyed when not empty");
				vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
				vkDestroyCommandPool(device, cmdPool, nullptr);
				device = VK_NULL_HANDLE;
			}
		}

		void CommandList::begin() {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			VkCommandBufferBeginInfo cbbi{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			};
			vkBeginCommandBuffer(cmd, &cbbi);
		}

		void CommandList::end() {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			vkEndCommandBuffer(cmd);
		}

		MappedStagingMemory CommandList::mapMemoryStaged(BufferHandle copyDst, size_t size, size_t dstOffset) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(size <= STAGING_BUFFER_POOL_BUFFER_SIZE, "Currently uploads over a size of 67.108.864 bytes are not supported by the uploadToBuffer function. Please use a staging buffer.");
			if (usedStagingBuffers.empty() || usedStagingBuffers.back().getLeftOverSize() < size) {
				usedStagingBuffers.push_back(stagingBufferPool.lock()->getStagingBuffer());
			}

			operationsInProgress += 1;

			auto& stagingBuffer = usedStagingBuffers.back();

			auto srcOffset = stagingBuffer.usedUpSize;

			u8* bufferHostPtr = (u8*)stagingBuffer.buffer->mapMemory();

			auto ret = MappedStagingMemory{ (void*)(bufferHostPtr + srcOffset), size, stagingBuffer.buffer };

			stagingBuffer.usedUpSize += size;
			stagingBuffer.usedUpSize = roundUpToMultipleOf128(stagingBuffer.usedUpSize);

			BufferToBufferCopyInfo btbCopyInfo{
				.src = stagingBuffer.buffer,
				.dst = copyDst,
				.region = BufferCopyRegion{
					.srcOffset = srcOffset,
					.dstOffset = dstOffset,
					.size = size,
				}
			};
			copyBufferToBuffer(btbCopyInfo);

			return ret;
		}
		
		void CommandList::unmapMemoryStaged(MappedStagingMemory& mappedstagingMemory) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(mappedstagingMemory.buffer->isMemoryMapped(), "the given mappedStagingMemory has allready been unmapped.");
			operationsInProgress -= 1;
			mappedstagingMemory.buffer->unmapMemory();
		}

		void CommandList::copyHostToBuffer(HostToBufferCopyInfo copyInfo) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(copyInfo.size <= STAGING_BUFFER_POOL_BUFFER_SIZE, "Currently uploads over a size of 67.108.864 bytes are not supported by the uploadToBuffer function. Please use a staging buffer.");
			if (usedStagingBuffers.empty() || usedStagingBuffers.back().getLeftOverSize() < copyInfo.size) {
				usedStagingBuffers.push_back(stagingBufferPool.lock()->getStagingBuffer());
			}

			auto& stagingBuffer = usedStagingBuffers.back();

			auto offset = stagingBuffer.usedUpSize;

			// memory is currently automaticly unmapped in Queue::submit
			stagingBuffer.buffer->upload(copyInfo.src, copyInfo.size, offset);

			stagingBuffer.usedUpSize += copyInfo.size;
			stagingBuffer.usedUpSize = roundUpToMultipleOf128(stagingBuffer.usedUpSize);

			BufferToBufferCopyInfo btbCopyInfo{
				.src = stagingBuffer.buffer,
				.dst = copyInfo.dst,
				.region = BufferCopyRegion{
					.srcOffset = offset,
					.dstOffset = copyInfo.dstOffset,
					.size = copyInfo.size,
				}
			};
			copyBufferToBuffer(btbCopyInfo);
		}

		void CommandList::copyHostToImage(HostToImageCopyInfo copyInfo) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(copyInfo.size <= STAGING_BUFFER_POOL_BUFFER_SIZE, "Currently uploads over a size of 67.108.864 bytes are not supported by the copyHostToImage function. Please use a staging buffer.");
			if (usedStagingBuffers.empty() || usedStagingBuffers.back().getLeftOverSize() < copyInfo.size) {
				usedStagingBuffers.push_back(stagingBufferPool.lock()->getStagingBuffer());
			}

			auto& stagingBuffer = usedStagingBuffers.back();

			auto offset = stagingBuffer.usedUpSize;

			stagingBuffer.buffer->upload(copyInfo.src, copyInfo.size, offset);

			stagingBuffer.usedUpSize += copyInfo.size;
			stagingBuffer.usedUpSize = roundUpToMultipleOf128(stagingBuffer.usedUpSize);

			BufferToImageCopyInfo btiCopy{
				.src = stagingBuffer.buffer,
				.dst = copyInfo.dst,
				.srcOffset = offset,
				.subRessourceLayers = copyInfo.dstImgSubressource,
				.size = copyInfo.size,
			};
			copyBufferToImage(btiCopy);
		}

		void CommandList::copyHostToImageSynced(HostToImageCopySyncedInfo copySyncedInfo) {
			ImageBarrier firstBarrier{
				.awaitedAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
				.awaitedStages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR,
				.waitingAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR,
				.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
				.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.image = copySyncedInfo.dst,
			};
			insertBarriers({},{&firstBarrier, 1});

			copyHostToImage({
				.src = copySyncedInfo.src,
				.dst = copySyncedInfo.dst, 
				.dstImgSubressource = copySyncedInfo.dstImgSubressource,
				.size = copySyncedInfo.size,
			});

			ImageBarrier secondBarrier{
				.awaitedAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
				.waitingAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR,
				.waitingStages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR,
				.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.layoutAfter = copySyncedInfo.dstFinalLayout,
				.image = copySyncedInfo.dst,
			};
			insertBarriers({},{&secondBarrier, 1});
		}

		void CommandList::copyMultiBufferToBuffer(BufferToBufferMultiCopyInfo copyInfo) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu.");
			DAXA_ASSERT_M(copyInfo.regions.size() > 0, "amount of copy regions must be greater than 0.");
			for (int i = 0; i < copyInfo.regions.size(); i++) {
				DAXA_ASSERT_M(copyInfo.src->getSize() >= copyInfo.regions[i].size + copyInfo.regions[i].srcOffset, "ERROR: src buffer is smaller than the region that shouly be copied!");
				DAXA_ASSERT_M(copyInfo.dst->getSize() >= copyInfo.regions[i].size + copyInfo.regions[i].dstOffset, "ERROR: dst buffer is smaller than the region that shouly be copied!");
			}
			usedBuffers.push_back(copyInfo.src);
			usedBuffers.push_back(copyInfo.dst);
			vkCmdCopyBuffer(cmd, copyInfo.src->getVkBuffer(), copyInfo.dst->getVkBuffer(), copyInfo.regions.size(), (VkBufferCopy*)copyInfo.regions.data());	// THIS COULD BREAK ON ABI CHANGE
		}

		void CommandList::copyBufferToBuffer(BufferToBufferCopyInfo copyInfo) {
			BufferToBufferMultiCopyInfo btbMultiCopy{
				.src = copyInfo.src,
				.dst = copyInfo.dst,
				.regions = { &copyInfo.region, 1 }
			};
			copyMultiBufferToBuffer(btbMultiCopy);
		}

		void CommandList::copyBufferToImage(BufferToImageCopyInfo copyInfo) {
			usedBuffers.push_back(copyInfo.src);
			usedImages.push_back(copyInfo.dst);

			VkImageSubresourceLayers imgSubRessource = copyInfo.subRessourceLayers.value_or(VkImageSubresourceLayers{
				.aspectMask = copyInfo.dst->getVkAspect(),
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = copyInfo.dst->getVkArrayLayers(),
				});

			VkBufferImageCopy bufferImageCopy{
				.bufferOffset = copyInfo.srcOffset,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = imgSubRessource,
				.imageOffset = 0,
				.imageExtent = copyInfo.dst->getVkExtent(),
			};

			vkCmdCopyBufferToImage(cmd, copyInfo.src->getVkBuffer(), copyInfo.dst->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
		}

		void CommandList::copyImageToImage(ImageToImageCopyInfo copyInfo) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");

			VkImageCopy copy{
				.dstOffset = copyInfo.dstOffset,
				.extent = copyInfo.size,
				.srcOffset = copyInfo.srcOffset,
				.srcSubresource = {
					.aspectMask = copyInfo.src->getVkAspect(),
					.baseArrayLayer = 0,
					.layerCount = 1,
					.mipLevel = 0,
				},
				.dstSubresource = {
					.aspectMask = copyInfo.dst->getVkAspect(),
					.baseArrayLayer = 0,
					.layerCount = 1,
					.mipLevel = 0,
				}
			};

			vkCmdCopyImage(cmd, copyInfo.src->getVkImage(), copyInfo.srcLayout, copyInfo.dst->getVkImage(), copyInfo.dstLayout, 1, &copy);
			usedImages.push_back(std::move(copyInfo.src));
			usedImages.push_back(std::move(copyInfo.dst));
		}

		void CommandList::copyImageToImageSynced(ImageToImageCopySyncedInfo copySyncedInfo) {
			insertImageBarriers(std::array{
				ImageBarrier{ 
					.image = copySyncedInfo.src, 
					.awaitedAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR, 
					.awaitedStages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR,
					.waitingAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR, 
					.layoutBefore = copySyncedInfo.srcLayoutBeforeAndAfter , 
					.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL 
				},
				ImageBarrier{ 
					.image = copySyncedInfo.dst, 
					.awaitedAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR, 
					.awaitedStages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR,
					.waitingAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR, 
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
					.image = std::move(copySyncedInfo.src), 
					.awaitedAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR, 
					.waitingAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR, 
					.waitingStages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR,
					.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
					.layoutAfter = copySyncedInfo.srcLayoutBeforeAndAfter 
				},
				ImageBarrier{ 
					.image = std::move(copySyncedInfo.dst), 
					.awaitedAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
					.waitingAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR, 
					.waitingStages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR,
					.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
					.layoutAfter = copySyncedInfo.dstFinalLayout
				},
			});
		}

		void CommandList::dispatch(u32 groupCountX, u32 groupCountY, u32 grpupCountZ) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(boundPipeline.value().bindPoint == VK_PIPELINE_BIND_POINT_COMPUTE, "can not dispatch compute commands with out a bound compute pipeline.");
			vkCmdDispatch(cmd, groupCountX, groupCountY, grpupCountZ);
		}

		void CommandList::bindVertexBuffer(u32 binding, BufferHandle buffer, size_t bufferOffset) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(buffer, "invalid buffer handle");
			auto vkBuffer = buffer->getVkBuffer();
			vkCmdBindVertexBuffers(cmd, binding, 1, &vkBuffer, &bufferOffset);
			usedBuffers.push_back(buffer);
		}

		void CommandList::bindIndexBuffer(BufferHandle buffer, size_t bufferOffset, VkIndexType indexType) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(buffer, "invalid buffer handle");
			auto vkBuffer = buffer->getVkBuffer();
			vkCmdBindIndexBuffer(cmd, vkBuffer, bufferOffset, indexType);
			usedBuffers.push_back(buffer);
		}

		void CommandList::beginRendering(BeginRenderingInfo ri) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			operationsInProgress += 1;
			for (int i = 0; i < ri.colorAttachments.size(); i++) {
				usedImages.push_back(ri.colorAttachments[i].image);

				renderAttachmentBuffer.push_back(VkRenderingAttachmentInfoKHR{
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
					.pNext = nullptr,
					.imageView = ri.colorAttachments[i].image->getVkView(),
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
						.imageView = ri.depthAttachment->image->getVkView(),
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
						.imageView = ri.stencilAttachment->image->getVkView(),
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
				renderInfo.renderArea.extent.width = ri.colorAttachments[0].image->getVkExtent().width;
				renderInfo.renderArea.extent.height = ri.colorAttachments[0].image->getVkExtent().height;
			}
			else if (ri.depthAttachment != nullptr) {
				renderInfo.renderArea.extent.width = ri.depthAttachment->image->getVkExtent().width;
				renderInfo.renderArea.extent.height = ri.depthAttachment->image->getVkExtent().height;
			}
			else if (ri.stencilAttachment != nullptr) {
				renderInfo.renderArea.extent.width = ri.stencilAttachment->image->getVkExtent().width;
				renderInfo.renderArea.extent.height = ri.stencilAttachment->image->getVkExtent().height;
			}	// otherwise let it be zero, as we dont render anything anyways

			renderInfo.layerCount = 1;	// Not sure what this does

			renderInfo.colorAttachmentCount = ri.colorAttachments.size();
			renderInfo.pColorAttachments = renderAttachmentBuffer.data();
			renderInfo.pDepthAttachment = depthAttachmentInfo.has_value() ? &depthAttachmentInfo.value() : nullptr;
			renderInfo.pStencilAttachment = stencilAttachmentInfo.has_value() ? &stencilAttachmentInfo.value() : nullptr;

			this->vkCmdBeginRenderingKHR(cmd, (VkRenderingInfoKHR*)&renderInfo);

			renderAttachmentBuffer.clear();

			setViewport(VkViewport{
				.x = 0,
				.y = 0,
				.width = (f32)renderInfo.renderArea.extent.width,
				.height = (f32)renderInfo.renderArea.extent.height,
				.minDepth = 0,
				.maxDepth = 1,
			});
		}
		void CommandList::endRendering() {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			operationsInProgress -= 1;
			this->vkCmdEndRenderingKHR(cmd);
			boundPipeline = std::nullopt;
		}

		void CommandList::bindPipeline(PipelineHandle& pipeline) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			vkCmdBindPipeline(cmd, pipeline->getVkBindPoint(), pipeline->getVkPipeline());
			usedGraphicsPipelines.push_back(pipeline);

			boundPipeline = BoundPipeline{
				.bindPoint = pipeline->getVkBindPoint(),
				.layout = pipeline->getVkPipelineLayout(),
			};
		}

		void CommandList::reset() {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(operationsInProgress == 0, "can not reset command list with recordings in progress");
			empty = true;
			vkResetCommandPool(device, cmdPool, VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
			usedBuffers.clear();
			usedGraphicsPipelines.clear();
			usedImages.clear();
			usedSets.clear();
			boundPipeline.reset();
			usedStagingBuffers.clear();
		}

		void CommandList::setViewport(VkViewport const& viewport) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			vkCmdSetViewport(cmd, 0, 1, &viewport); 
			VkRect2D scissor{
				.offset = { 0, 0 },
				.extent = { (u32)viewport.width, (u32)viewport.height },
			};
			setScissor(scissor);
		}

		void CommandList::setScissor(VkRect2D const& scissor) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			vkCmdSetScissor(cmd, 0, 1, &scissor);
		}
		
		void CommandList::draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(boundPipeline.has_value(), "can not draw sets if there is no pipeline bound");
			vkCmdDraw(cmd, vertexCount, instanceCount, firstVertex, firstInstance);
		}

		void CommandList::drawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstIntance) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(boundPipeline.has_value(), "can not draw sets if there is no pipeline bound");
			vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstIntance);
		}

		void CommandList::bindSet(u32 setBinding, BindingSetHandle set) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			DAXA_ASSERT_M(boundPipeline.has_value(), "can not bind sets if there is no pipeline bound");
			vkCmdBindDescriptorSets(cmd, boundPipeline->bindPoint, boundPipeline->layout, setBinding, 1, &set->set, 0, nullptr);
			usedSets.push_back(std::move(set));
		}

		void CommandList::bindSetPipelineIndependant(u32 setBinding, BindingSetHandle set, VkPipelineBindPoint bindPoint, VkPipelineLayout layout) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
			vkCmdBindDescriptorSets(cmd, bindPoint, layout, setBinding, 1, &set->set, 0, nullptr);
			usedSets.push_back(std::move(set));
		}

		void CommandList::insertBarriers(std::span<MemoryBarrier> memBarriers, std::span<ImageBarrier> imgBarriers) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not change command list, that is currently used on gpu");
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
					.srcStageMask = barrier.awaitedStages,
					.srcAccessMask = barrier.awaitedAccess,
					.dstStageMask = barrier.waitingStages,
					.dstAccessMask = barrier.waitingAccess,
				};
			}

			//for (auto& barrier : bufBarriers) {
			//	DAXA_ASSERT_M(bufBarrierBufferSize < 32, "can only insert 32 barriers of one kind in a single insertBarriers call");
			//	bufBarrierBuffer[bufBarrierBufferSize] = VkBufferMemoryBarrier2KHR{
			//		.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2_KHR,
			//		.pNext = nullptr,
			//		.srcStageMask = barrier.awaitedStages,
			//		.srcAccessMask = barrier.awaitedAccess,
			//		.dstStageMask = barrier.waitingStages,
			//		.dstAccessMask = barrier.waitingAccess,
			//		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			//		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			//		.buffer = barrier.buffer->getVkBuffer(),
			//		.offset = barrier.offset,
			//		.size = barrier.buffer->getSize(),
			//	};
//
			//	usedBuffers.push_back(barrier.buffer);
//
			//	if (barrier.size.has_value()) {
			//		bufBarrierBuffer[bufBarrierBufferSize].size = *barrier.size;
			//	}
//
			//	bufBarrierBufferSize++;
			//}

			for (auto& barrier : imgBarriers) {
				DAXA_ASSERT_M(bufBarrierBufferSize < 32, "can only insert 32 barriers of one kind in a single insertBarriers call");

				imgBarrierBuffer[imgBarrierBufferSize] = VkImageMemoryBarrier2KHR{
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR,
					.pNext = nullptr,
					.srcStageMask = barrier.awaitedStages,
					.srcAccessMask = barrier.awaitedAccess,
					.dstStageMask = barrier.waitingStages,
					.dstAccessMask = barrier.waitingAccess,
					.oldLayout = barrier.layoutBefore,
					.newLayout = barrier.layoutAfter,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.image = barrier.image->getVkImage(),
					.subresourceRange = barrier.subRange.value_or(VkImageSubresourceRange{
						.aspectMask = barrier.image->getVkAspect(),
						.baseMipLevel = 0,
						.levelCount = barrier.image->getVkMipmapLevels(),
						.baseArrayLayer = 0,
						.layerCount = barrier.image->getVkArrayLayers(),
					})
				};

				usedImages.push_back(barrier.image);

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

			this->vkCmdPipelineBarrier2KHR(cmd, &dependencyInfo);
		}

		void CommandList::insertFullMemoryBarrier() {
			MemoryBarrier barrier{
				.awaitedAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
				.awaitedStages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR,
				.waitingAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR,
				.waitingStages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR,
			};
			insertBarriers({&barrier,1}, {});
		}

		void CommandListHandle::cleanup() {
			if (list && list.use_count() == 1) {
				if (auto recyclingSharedData = list->recyclingData.lock()) {
					list->reset();
					auto lock = std::unique_lock(recyclingSharedData->mut);
					recyclingSharedData->zombies.push_back(std::move(list));
				}
				list = {};
			}
		}

		CommandListHandle::~CommandListHandle() {
			cleanup();
		}

		CommandListHandle& CommandListHandle::operator=(CommandListHandle&& other) noexcept {
			cleanup();
			list = std::move(other.list);
			return *this;
		}

		CommandListHandle& CommandListHandle::operator=(CommandListHandle const& other) {
			cleanup();
			list = other.list;
			return *this;
		}
		
		CommandListHandle::CommandListHandle(std::shared_ptr<CommandList> list)
			: list{ list }
		{}
	}
}
