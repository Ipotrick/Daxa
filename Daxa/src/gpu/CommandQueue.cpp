#include "CommandQueue.hpp"
#include "Instance.hpp"
#include "backend/DeviceBackend.hpp"
#include "backend/CommandListBackend.hpp"

namespace daxa {
	CommandQueue::CommandQueue(
		std::shared_ptr<DeviceBackend> deviceBackend, 
		VkQueue queue, 
		u32 queueFamilyIndex, 
		std::shared_ptr<StagingBufferPool> uploadStagingBufferPool, 
		std::shared_ptr<StagingBufferPool> downloadStagingBufferPool, 
		CommandQueueCreateInfo const& ci)
		: deviceBackend{ std::move(deviceBackend) }
		, queue{ queue }
		, queueFamilyIndex{ queueFamilyIndex }
		, bWaitForBatchesToComplete{ ci.batchCount > 0 }
		, cmdListRecyclingSharedData{ std::make_shared<CommandListRecyclingSharedData>() }
		, uploadStagingBufferPool{ std::move(uploadStagingBufferPool) }
	{
		this->batches.resize(std::max(u32(1), ci.batchCount));

		if (ci.debugName) {
			this->debugName = ci.debugName;
			if (instance->pfnSetDebugUtilsObjectNameEXT) {
				VkDebugUtilsObjectNameInfoEXT imageNameInfo{
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					.pNext = NULL,
					.objectType = VK_OBJECT_TYPE_QUEUE,
					.objectHandle = (uint64_t)queue,
					.pObjectName = this->debugName.c_str(),
				};
				instance->pfnSetDebugUtilsObjectNameEXT(this->deviceBackend->device.device, &imageNameInfo);
			}
		}
	}

	CommandQueue::~CommandQueue() {
		waitIdle();
	}

	void CommandQueue::submit(SubmitInfo si) {
		for (auto& cmdList : si.commandLists) {
			DAXA_ASSERT_M(cmdList.get()->finalized, "can only submit finalized command lists");
			DAXA_ASSERT_M(cmdList.get()->recyclingData.lock().get() == cmdListRecyclingSharedData.get(), "comand lists can only be submitted to the queue they were created from");
			for (auto& sbuffer : cmdList.get()->usedUploadStagingBuffers) {
				DAXA_ASSERT_M(!(*sbuffer.buffer).isMemoryMapped(), "can not submit command list. Some Buffers used in the command list have mapped memory, all memory to used buffers need to be unmapped before a submit.");
			}

			submitCommandBufferBuffer.push_back(cmdList.get()->cmd);
			cmdList.get()->usesOnGPU += 1;
		
			for (auto& set : cmdList.get()->usedSets) {
				set->usesOnGPU += 1;
			}
			
			for (auto& [timelineSema, signalValue] : cmdList.get()->usedTimelines) {
				this->submitSemaphoreSignalBuffer.push_back(timelineSema->getVkSemaphore());
				this->submitSemaphoreSignalValueBuffer.push_back(signalValue);
			}
		}

		for (auto& signal : si.waitOnSignals) {
			this->submitSemaphoreWaitOnBuffer.push_back(signal->getVkSemaphore());
			this->submitSemaphoreWaitOnValueBuffer.push_back(0);
		}
		for (auto& [timelineSema, waitValue] : si.waitOnTimelines) {
			this->submitSemaphoreWaitOnBuffer.push_back(timelineSema->getVkSemaphore());
			this->submitSemaphoreWaitOnValueBuffer.push_back(waitValue);
		}
		for (auto& signal : si.signalOnCompletion) {
			this->submitSemaphoreSignalBuffer.push_back(signal->getVkSemaphore());
			this->submitSemaphoreSignalValueBuffer.push_back(0);
		}
		for (auto& [timelineSema, signalValue] : si.signalTimelines) {
			this->submitSemaphoreSignalBuffer.push_back(timelineSema->getVkSemaphore());
			this->submitSemaphoreSignalValueBuffer.push_back(signalValue);
		}

		auto thisSubmitTimelineSema = getNextTimeline();
		auto thisSubmitTimelineFinishCounter = thisSubmitTimelineSema->getCounter() + 1;
		this->submitSemaphoreSignalBuffer.push_back(thisSubmitTimelineSema->getVkSemaphore());
		this->submitSemaphoreSignalValueBuffer.push_back(thisSubmitTimelineFinishCounter);

		VkTimelineSemaphoreSubmitInfo timelineSI{
			.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreValueCount = (u32)submitSemaphoreWaitOnValueBuffer.size(),
			.pWaitSemaphoreValues = submitSemaphoreWaitOnValueBuffer.data(),
			.signalSemaphoreValueCount = (u32)submitSemaphoreSignalValueBuffer.size(),
			.pSignalSemaphoreValues = submitSemaphoreSignalValueBuffer.data(),
		};

		VkPipelineStageFlags pipelineStages = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;	// TODO maybe remove this
		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = &timelineSI,
			.waitSemaphoreCount = (u32)submitSemaphoreWaitOnBuffer.size(),
			.pWaitSemaphores = submitSemaphoreWaitOnBuffer.data(),
			.pWaitDstStageMask = &pipelineStages,
			.commandBufferCount = (u32)submitCommandBufferBuffer.size(),
			.pCommandBuffers = submitCommandBufferBuffer.data() ,
			.signalSemaphoreCount = (u32)submitSemaphoreSignalBuffer.size(),
			.pSignalSemaphores = submitSemaphoreSignalBuffer.data(),
		};
		DAXA_CHECK_VK_RESULT_M(vkQueueSubmit(queue, 1, &submitInfo, nullptr), "vkQueueSubmit failed");

		PendingSubmit pendingSubmit{
			.cmdLists = { si.commandLists },
			.timelineSema = std::move(thisSubmitTimelineSema),
			.finishCounter = thisSubmitTimelineFinishCounter,
		};
		batches[0].push_back(std::move(pendingSubmit));

		submitSemaphoreWaitOnBuffer.clear();
		submitSemaphoreWaitOnBuffer.clear();
		submitSemaphoreSignalBuffer.clear();
		submitSemaphoreSignalValueBuffer.clear();
		submitCommandBufferBuffer.clear();
	}

	TimelineSemaphoreHandle CommandQueue::getNextTimeline() {
		if (unusedTimelines.empty()) {
			unusedTimelines.push_back(TimelineSemaphoreHandle{ std::make_shared<TimelineSemaphore>(deviceBackend, TimelineSemaphoreCreateInfo{.debugName = "queue timline"}) });
		}
		auto timeline = std::move(unusedTimelines.back());
		unusedTimelines.pop_back();
		return timeline;
	}

	void CommandQueue::submitBlocking(SubmitInfo submitInfo) {
		submit(submitInfo);
		batches[0].back().timelineSema->wait(batches[0].back().finishCounter);
	}

	void CommandQueue::checkForFinishedSubmits() {
		for (auto& batch : batches) {
			for (auto iter = batch.begin(); iter != batch.end();) {
				if (iter->timelineSema->getCounter() >= iter->finishCounter) {
					while (!iter->cmdLists.empty()) {
						auto list = std::move(iter->cmdLists.back());
						iter->cmdLists.pop_back();
						for (auto& set : list.get()->usedSets) {
							set->usesOnGPU -= 1;
						}
						list.get()->usesOnGPU -= 1;
					}
					unusedTimelines.push_back(std::move(iter->timelineSema));
					iter = batch.erase(iter);
				}
				else {
					++iter;
				}
			}
		}
	}

	void CommandQueue::present(SwapchainImage&& img, SignalHandle& waitOnSignal) {
		auto image = std::move(img);
		auto sema = waitOnSignal->getVkSemaphore();
		VkPresentInfoKHR presentInfo{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = nullptr,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &sema,
			.swapchainCount = 1,
			.pSwapchains = &image.swapchain,
			.pImageIndices = &image.imageIndex,
		};
		DAXA_CHECK_VK_RESULT_M(vkQueuePresentKHR(queue, &presentInfo), "failed vkQueuePresentKHR");
	}

	void CommandQueue::waitIdle() {
		for (auto& batch : batches) {
			for (auto& pending : batch) {
				pending.timelineSema->wait(pending.finishCounter);
			}
		}
	}

	void CommandQueue::nextBatch() {
		auto back = std::move(batches.back());
		batches.pop_back();
		batches.push_front(std::move(back));
		if (bWaitForBatchesToComplete) {
			for (auto& pending : batches[0]) {
				pending.timelineSema->wait(pending.finishCounter);
			}
		}
	}

	CommandListHandle CommandQueue::getCommandList(CommandListFetchInfo const& fi) {
		auto list = std::move(getNextCommandList());
		list.get()->setDebugName(fi.debugName);
		{
			std::unique_lock lock(deviceBackend->graveyard.mtx);
			deviceBackend->graveyard.activeZombieLists.push_back(list.get()->zombies);
		}
		list.get()->begin();
		return std::move(list);
	}

	CommandListHandle CommandQueue::getNextCommandList() {
		if (unusedCommandLists.empty()) {
			auto lock = std::unique_lock(cmdListRecyclingSharedData->mut);
			while (!cmdListRecyclingSharedData->zombies.empty()) {
				unusedCommandLists.push_back(CommandListHandle{std::move(cmdListRecyclingSharedData->zombies.back())});
				cmdListRecyclingSharedData->zombies.pop_back();
			}
		}

		if (unusedCommandLists.empty()) {
			// we have no command lists left, we need to create new ones:
			unusedCommandLists.push_back(CommandListHandle{ std::make_shared<CommandListBackend>() });
			CommandListBackend& list = *unusedCommandLists.back().get();
			list.deviceBackend = deviceBackend;
			list.uploadStagingBufferPool = uploadStagingBufferPool;
			list.recyclingData = cmdListRecyclingSharedData;

			VkCommandPoolCreateInfo commandPoolCI{
				.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.pNext = nullptr,
				.queueFamilyIndex = queueFamilyIndex,
			};
			DAXA_CHECK_VK_RESULT(vkCreateCommandPool(deviceBackend->device.device, &commandPoolCI, nullptr, &list.cmdPool));

			VkCommandBufferAllocateInfo commandBufferAllocateInfo{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext = nullptr,
				.commandPool = list.cmdPool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};
			DAXA_CHECK_VK_RESULT(vkAllocateCommandBuffers(deviceBackend->device.device, &commandBufferAllocateInfo, &list.cmd));
		} 
		auto ret = std::move(unusedCommandLists.back());
		unusedCommandLists.pop_back();
		return std::move(ret);
	}
}