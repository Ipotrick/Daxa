#include "CommandList.hpp"

#include "common.hpp"

namespace daxa {
	namespace gpu {
		DAXA_DEFINE_TRIVIAL_MOVE(CommandList)

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
				printf("command list complete destruction!\n");
				std::memset(this, 0, sizeof(CommandList));
			}
		}

		void CommandList::begin() {
			operationsInProgress += 1;
			VkCommandBufferBeginInfo cbbi{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			};
			vkBeginCommandBuffer(cmd, &cbbi);
		}

		void CommandList::end() {
			operationsInProgress -= 1;
			vkEndCommandBuffer(cmd);
		}

		void CommandList::bindVertexBuffer(u32 binding, BufferHandle buffer, size_t bufferOffset) {
			vkCmdBindVertexBuffers(cmd, binding, 1, &buffer->buffer, &bufferOffset);
		}

		void CommandList::beginRendering(BeginRenderingInfo ri) {
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
						.storeOp = ri.depthAttachment->storeOp
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
						.storeOp = ri.stencilAttachment->storeOp
					}
				};

			VkRenderingInfoKHR renderInfo{};
			renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
			renderInfo.pNext = nullptr;
			if (ri.renderArea) {
				renderInfo.renderArea = *ri.renderArea;
			}
			else if (ri.colorAttachments.size() > 0) {
				renderInfo.renderArea.extent.width = ri.colorAttachments[0].image->getExtent().width;
				renderInfo.renderArea.extent.height = ri.colorAttachments[0].image->getExtent().height;
			}
			else if (ri.depthAttachment != nullptr) {
				renderInfo.renderArea.extent.width = ri.depthAttachment->image->getExtent().width;
				renderInfo.renderArea.extent.height = ri.depthAttachment->image->getExtent().height;
			}
			else if (ri.stencilAttachment != nullptr) {
				renderInfo.renderArea.extent.width = ri.stencilAttachment->image->getExtent().width;
				renderInfo.renderArea.extent.height = ri.stencilAttachment->image->getExtent().height;
			}	// otherwise let it be zero, as we dont render anything anyways

			renderInfo.layerCount = 1;	// Not sure what this does

			renderInfo.colorAttachmentCount = ri.colorAttachments.size();
			renderInfo.pColorAttachments = renderAttachmentBuffer.data();
			renderInfo.pDepthAttachment = depthAttachmentInfo.has_value() ? &depthAttachmentInfo.value() : nullptr;
			renderInfo.pStencilAttachment = stencilAttachmentInfo.has_value() ? &stencilAttachmentInfo.value() : nullptr;

			this->vkCmdBeginRenderingKHR(cmd, (VkRenderingInfoKHR*)&renderInfo);

			renderAttachmentBuffer.clear();
		}
		void CommandList::endRendering() {
			operationsInProgress -= 1;
			this->vkCmdEndRenderingKHR(cmd);
			boundPipeline = std::nullopt;
		}

		void CommandList::bindPipeline(GraphicsPipelineHandle graphicsPipeline) {
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getVkPipeline());
			usedGraphicsPipelines.push_back(graphicsPipeline);

			boundPipeline = BoundPipeline{
				.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.layout = graphicsPipeline->getVkPipelineLayout(),
			};
		}

		void CommandList::reset() {
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
			vkCmdSetViewport(cmd, 0, 1, &viewport); 
			VkRect2D scissor{
				.offset = { 0, 0 },
				.extent = { (u32)viewport.width, (u32)viewport.height },
			};
			setScissor(scissor);
		}

		void CommandList::setScissor(VkRect2D const& scissor) {
			vkCmdSetScissor(cmd, 0, 1, &scissor);
		}

		void CommandList::draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) {
			vkCmdDraw(cmd, vertexCount, instanceCount, firstVertex, firstInstance);
		}

		void CommandList::copyBufferToBufferMulti(BufferHandle src, BufferHandle dst, std::span<VkBufferCopy> copyRegions) {
			DAXA_ASSERT_M(copyRegions.size() > 0, "ERROR: tried copying 0 regions from buffer to buffer, this is a bug!");
			for (int i = 0; i < copyRegions.size(); i++) {
				DAXA_ASSERT_M(src->getSize() >= copyRegions[i].size + copyRegions[i].srcOffset, "ERROR: src buffer is smaller than the region that shouly be copied!");
				DAXA_ASSERT_M(dst->getSize() >= copyRegions[i].size + copyRegions[i].dstOffset, "ERROR: dst buffer is smaller than the region that shouly be copied!");
			}
			usedBuffers.push_back(src);
			usedBuffers.push_back(dst);
			vkCmdCopyBuffer(cmd, src->getVkBuffer(), dst->getVkBuffer(), copyRegions.size(), copyRegions.data());
		}

		void CommandList::updateSetImages(BindingSetHandle& set, u32 binding, std::span<std::pair<ImageHandle, VkImageLayout>> images, u32 descriptorArrayOffset) {
			DAXA_ASSERT_M(!set->bInUseOnGPU, "can not update binding set while it is used on gpu");
			for (auto& [image, layout] : images) {
				VkSampler sampler = VK_NULL_HANDLE;
				switch (set->description->layoutBindings.bindings[binding].descriptorType) {
				case VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
					// TODO REPLACE THIS WITH AN ACTUAL SAMPLER!
					sampler = VK_NULL_HANDLE;
					break;
				case VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
					sampler = VK_NULL_HANDLE;
					break;
				case VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
					sampler = VK_NULL_HANDLE;
					break;
				default:
					DAXA_ASSERT_M(false, "binding set binding mismatch");
				}

				imageInfoBuffer.push_back(VkDescriptorImageInfo{
					.sampler = sampler,
					.imageView = image->view,
					.imageLayout = layout,
					});

				// update the handles inside the set
				u32 bindingSetIndex = set->description->bindingToHandleVectorIndex[binding];
				set->handles[bindingSetIndex] = image;
			}

			VkWriteDescriptorSet write{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = set->set,
				.dstBinding = binding,
				.dstArrayElement = descriptorArrayOffset,
				.descriptorCount = (u32)imageInfoBuffer.size(),
				.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = imageInfoBuffer.data()
			};

			vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

			imageInfoBuffer.clear();
		}

		void CommandList::updateSetImage(BindingSetHandle& set, u32 binding, ImageHandle image, VkImageLayout layout) {
			std::pair pair{ image,layout };
			updateSetImages(set, binding, { &pair, 1 });
		}

		void CommandList::updateSetBuffers(BindingSetHandle& set, u32 binding, std::span<BufferHandle> buffers, u32 offset) {
			DAXA_ASSERT_M(!set->bInUseOnGPU, "can not update binding set, that is still in use on the gpu");
			for (auto& buffer : buffers) {
				bufferInfoBuffer.push_back(VkDescriptorBufferInfo{
					.buffer = buffer->buffer,
					.offset = 0,									// TODO Unsure what to put here
					.range = buffer->size,
				});

				// update the handles inside the set
				u32 bindingSetIndex = set->description->bindingToHandleVectorIndex[binding];
				set->handles[bindingSetIndex] = buffer;
			}
			
			VkWriteDescriptorSet write{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = set->set,
				.dstBinding = binding,
				.dstArrayElement = offset,
				.descriptorCount = (u32)bufferInfoBuffer.size(),
				.descriptorType = set->description->layoutBindings.bindings[binding].descriptorType,
				.pBufferInfo = bufferInfoBuffer.data(),
			};

			vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

			bufferInfoBuffer.clear();
		}

		void CommandList::updateSetBuffer(BindingSetHandle& set, u32 binding, BufferHandle buffer) {
			updateSetBuffers(set, binding, { &buffer, 1 });
		}

		void CommandList::bindSet(u32 setBinding, BindingSetHandle& set) {
			DAXA_ASSERT_M(boundPipeline.has_value(), "can not bind descriptor sets if there is no pipeline bound");
			vkCmdBindDescriptorSets(cmd, boundPipeline->bindPoint, boundPipeline->layout, setBinding, 1, &set->set, 0, nullptr);
			usedSets.push_back(set);
		}

		void CommandList::uploadToBuffer(void const* src, size_t size, BufferHandle dst, size_t dstOffset) {
			if (size > STAGING_BUFFER_POOL_BUFFER_SIZE) {
				DAXA_ASSERT_M(false, "Currently uploads over a size of 67.108.864 bytes are not supported by the uploadToBuffer function. Please use a staging buffer.");
			}
			else {
				if (usedStagingBuffers.empty() || usedStagingBuffers.back().getLeftOverSize() < size) {
					usedStagingBuffers.push_back(stagingBufferPool.lock()->getStagingBuffer());
				}

				auto& stagingBuffer = usedStagingBuffers.back();

				auto offset = stagingBuffer.usedUpSize;

				stagingBuffer.buffer->uploadFromHost(src, size, offset);

				stagingBuffer.usedUpSize += size;

				VkBufferCopy bufferCopyInfo{
					.srcOffset = offset,
					.dstOffset = dstOffset,
					.size = size,
				};

				copyBufferToBuffer(stagingBuffer.buffer, dst, bufferCopyInfo);
			}
		}

		void CommandList::insertBarriers(std::span<MemoryBarrier> memBarriers, std::span<BufferBarrier> bufBarriers, std::span<ImageBarrier> imgBarriers) {
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

			for (auto& barrier : bufBarriers) {
				DAXA_ASSERT_M(bufBarrierBufferSize < 32, "can only insert 32 barriers of one kind in a single insertBarriers call");
				bufBarrierBuffer[bufBarrierBufferSize] = VkBufferMemoryBarrier2KHR{
					.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2_KHR,
					.pNext = nullptr,
					.srcStageMask = barrier.awaitedStages,
					.srcAccessMask = barrier.awaitedAccess,
					.dstStageMask = barrier.waitingStages,
					.dstAccessMask = barrier.waitingAccess,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.buffer = barrier.buffer->buffer,
					.offset = barrier.offset,
					.size = barrier.buffer->size,
				};

				usedBuffers.push_back(barrier.buffer);

				if (barrier.size.has_value()) {
					bufBarrierBuffer[bufBarrierBufferSize].size = *barrier.size;
				}

				bufBarrierBufferSize++;
			}

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
					.image = barrier.image->image,
					.subresourceRange = barrier.subRange.value_or(VkImageSubresourceRange{
						.aspectMask = barrier.image->aspect,
						.baseMipLevel = 0,
						.levelCount = barrier.image->mipmapLevels,
						.baseArrayLayer = 0,
						.layerCount = barrier.image->arrayLayers,
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
				.bufferMemoryBarrierCount = bufBarrierBufferSize,
				.pBufferMemoryBarriers = bufBarrierBuffer.data(),
				.imageMemoryBarrierCount = imgBarrierBufferSize,
				.pImageMemoryBarriers = imgBarrierBuffer.data(),
			};

			this->vkCmdPipelineBarrier2KHR(cmd, &dependencyInfo);
		}
	}
}
