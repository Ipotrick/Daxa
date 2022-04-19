#include "CommandList.hpp"

#include "util.hpp"
#include "Instance.hpp"
#include "backend/DeviceBackend.hpp"
#include "backend/CommandListBackend.hpp"

namespace daxa {	
	void CommandListHandleStaticFunctionOverride::cleanup(std::shared_ptr<CommandListBackend>& value) {
		if (value && value.use_count() == 1) {
			if (auto recyclingSharedData = value->recyclingData.lock()) {
				value->reset();
				auto lock = std::unique_lock(recyclingSharedData->mut);
				std::shared_ptr<CommandListBackend> dummy{};
				std::swap(dummy, value);
				recyclingSharedData->zombies.push_back(std::move(dummy));
			}
			value = {};
		}
	}
	
	void CommandListHandle::finalize() {
		value->finalize();
	}
	MappedMemoryPointer CommandListHandle::mapMemoryStagedBuffer(BufferHandle copyDst, size_t size, size_t dstOffset) {
		return std::move(value->mapMemoryStagedBuffer(copyDst, size, dstOffset));
	}
	MappedMemoryPointer CommandListHandle::mapMemoryStagedImage(
		ImageHandle copyDst, 
		VkImageSubresourceLayers subressource, 
		VkOffset3D dstOffset, 
		VkExtent3D dstExtent
	) {
		return std::move(value->mapMemoryStagedImage(copyDst, subressource, dstOffset, dstExtent));
	}
	void CommandListHandle::multiCopyHostToBuffer(MultiHostToBufferCopyInfo const& ci) {
		value->multiCopyHostToBuffer(ci);
	}
	void CommandListHandle::singleCopyHostToBuffer(SingleCopyHostToBufferInfo const& ci) {
		value->singleCopyHostToBuffer(ci);
	}
	ToHostCopyFuture CommandListHandle::singleCopyBufferToHost(SingleBufferToHostCopyInfo const& ci) {
		return value->singleCopyBufferToHost(ci);
	}
	void CommandListHandle::multiCopyHostToImage(MultiHostToImageCopyInfo const& ci) {
		value->multiCopyHostToImage(ci);
	}
	void CommandListHandle::singleCopyHostToImage(SingleHostToImageCopyInfo const& ci) {
		value->singleCopyHostToImage(ci);
	}
	void CommandListHandle::multiCopyBufferToBuffer(MultiBufferToBufferCopyInfo const& ci) {
		value->multiCopyBufferToBuffer(ci);
	}
	void CommandListHandle::singleCopyBufferToBuffer(SingleBufferToBufferCopyInfo const& ci) {
		value->singleCopyBufferToBuffer(ci);
	}
	void CommandListHandle::multiCopyBufferToImage(MultiBufferToImageCopyInfo const& ci) {
		value->multiCopyBufferToImage(ci);
	}
	void CommandListHandle::singleCopyBufferToImage(SingleBufferToImageCopyInfo const& ci) {
		value->singleCopyBufferToImage(ci);
	}
	void CommandListHandle::multiCopyImageToImage(MultiImageToImageCopyInfo const& ci) {
		value->multiCopyImageToImage(ci);
	}
	void CommandListHandle::singleCopyImageToImage(SingleImageToImageCopyInfo const& ci) {
		value->singleCopyImageToImage(ci);
	}
	void CommandListHandle::dispatch(u32 groupCountX, u32 groupCountY, u32 grpupCountZ) {
		value->dispatch(groupCountX, groupCountY, grpupCountZ);
	}
	void CommandListHandle::beginRendering(BeginRenderingInfo ri) {
		value->beginRendering(ri);
	}
	void CommandListHandle::endRendering() {
		value->endRendering();
	}
	void CommandListHandle::bindPipeline(PipelineHandle& pipeline) {
		value->bindPipeline(pipeline);
	}
	void CommandListHandle::unbindPipeline() {
		value->unbindPipeline();
	}
	void CommandListHandle::setViewport(VkViewport const& viewport) {
		value->setViewport(viewport);
	}
	void CommandListHandle::setScissor(VkRect2D const& scissor) {
		value->setScissor(scissor);
	}
	void CommandListHandle::pushConstant(VkShaderStageFlags shaderStage, void const* data, u32 size, u32 offset) {
		value->pushConstant(shaderStage, data, size, offset);
	}
	void CommandListHandle::bindVertexBuffer(u32 binding, BufferHandle& buffer, size_t bufferOffset) {
		value->bindVertexBuffer(binding, buffer, bufferOffset);
	}
	void CommandListHandle::bindIndexBuffer(BufferHandle& buffer, size_t bufferOffset, VkIndexType indexType) {
		value->bindIndexBuffer(buffer, bufferOffset, indexType);
	}
	void CommandListHandle::bindSet(u32 setBinding, BindingSetHandle set) {
		value->bindSet(setBinding, set);
	}
	void CommandListHandle::bindSetPipelineIndependant(u32 setBinding, BindingSetHandle set, VkPipelineBindPoint bindPoint, VkPipelineLayout layout) {
		value->bindSetPipelineIndependant(setBinding, set, bindPoint, layout);
	}
	void CommandListHandle::draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) {
		value->draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}
	void CommandListHandle::drawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstIntance) {
		value->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstIntance);
	}
	void CommandListHandle::queueMemoryBarrier(MemoryBarrier const& memoryBarrier) {
		value->queueMemoryBarrier(memoryBarrier);
	}
	void CommandListHandle::queueImageBarrier(ImageBarrier const& memoryBarrier) {
		value->queueImageBarrier(memoryBarrier);
	}
	void CommandListHandle::insertQueuedBarriers() {
		value->insertQueuedBarriers();
	}
	void CommandListHandle::bindAll(u32 set) {
		value->bindAll(set);
	}
	VkCommandBuffer CommandListHandle::getVkCommandBuffer() {
		return value->getVkCommandBuffer();
	}
	VkCommandPool CommandListHandle::getVkCommandPool() {
		return value->getVkCommandPool();
	}
	std::string const& CommandListHandle::getDebugName() const {
		return value->getDebugName();
	}
}
