#include "Queue.hpp"

#include "common.hpp"

namespace daxa {
	namespace gpu {
		DAXA_DEFINE_TRIVIAL_MOVE(Queue)

		Queue::Queue(VkDevice device, VkQueue queue, std::shared_ptr<CommandListRecyclingSharedData> sharedData)
			: device{device}
			, queue{queue}
			, sharedData{sharedData}
		{}

		Queue::~Queue() {
			if (device) {
				waitForFlush();
				device = nullptr;
			}
		}

		void Queue::submit(SubmitInfo&& si) {
			//for (auto& cmdList : si.commandLists) {
			//	DAXA_ASSERT_M(cmdList.operationsInProgress == 0, "can not submit command list with recording in progress");
			//	submitCommandBufferBuffer.push_back(cmdList.cmd);
			//
			//	for (auto& buffer : cmdList.usedBuffers) {
			//		buffer->bInUseOnGPU = true;
			//	}
			//	for (auto& set : cmdList.usedSets) {
			//		set->bInUseOnGPU = true;
			//	}
			//}
			//
			//for (auto [timelineSema, waitValue] : si.waitOnTimelines) {
			//	this->submitSemaphoreWaitOnBuffer.push_back(timelineSema->getVkSemaphore());
			//	this->submitSemaphoreWaitOnValueBuffer.push_back(waitValue);
			//}
			//for (auto signal : si.waitOnSignals) {
			//	this->submitSemaphoreWaitOnBuffer.push_back(signal->getVkSemaphore());
			//	this->submitSemaphoreWaitOnValueBuffer.push_back(0);
			//}
			//for (auto [timelineSema, signalValue] : si.signalTimelines) {
			//	this->submitSemaphoreSignalBuffer.push_back(timelineSema->getVkSemaphore());
			//	this->submitSemaphoreSignalValueBuffer.push_back(signalValue);
			//}
			//for (auto signal : si.signalOnCompletion) {
			//	this->submitSemaphoreSignalBuffer.push_back(signal->getVkSemaphore());
			//	this->submitSemaphoreSignalValueBuffer.push_back(0);
			//}
			//

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
			vkQueueSubmit(queue, 1, &submitInfo, nullptr);

			auto thisSubmitTimelineSema = getNextTimeline();
			auto thisSubmitTimelineFinishCounter = thisSubmitTimelineSema.getCounter() + 1;
			this->submitSemaphoreSignalBuffer.push_back(thisSubmitTimelineSema.getVkSemaphore());
			this->submitSemaphoreSignalValueBuffer.push_back(thisSubmitTimelineFinishCounter);
			PendingSubmit pendingSubmit{
				.cmdLists = std::move(si.commandLists),
				.timelineSema = std::move(thisSubmitTimelineSema),
				.finishCounter = thisSubmitTimelineFinishCounter,
			};
			auto someMove = std::move(pendingSubmit);
			unfinishedSubmits.push_back(std::move(someMove));

			submitSemaphoreWaitOnBuffer.clear();
			submitSemaphoreWaitOnBuffer.clear();
			submitSemaphoreSignalBuffer.clear();
			submitSemaphoreSignalValueBuffer.clear();
			submitCommandBufferBuffer.clear();
		}

		TimelineSemaphore Queue::getNextTimeline() {
			if (unusedTimelines.empty()) {
				unusedTimelines.push_back(TimelineSemaphore{ device });
			}
			auto timeline = std::move(unusedTimelines.back());
			unusedTimelines.pop_back();
			return timeline;
		}

		void Queue::checkForFinishedSubmits() {
			for (auto iter = unfinishedSubmits.begin(); iter != unfinishedSubmits.end();) {
				auto iter2 = iter;
				if (iter->timelineSema.getCounter() >= iter->finishCounter) {
					while (!iter->cmdLists.empty()) {
						auto list = std::move(iter->cmdLists.back());
						iter->cmdLists.pop_back();
						for (auto& buffer : list.usedBuffers) {
							buffer->bInUseOnGPU = false;
						}
						for (auto& set : list.usedSets) {
							set->bInUseOnGPU = false;
						}
						list.reset();
			
						auto data = sharedData.lock();
						auto lock = std::unique_lock(data->mut);
						data->emptyCommandLists.push_back(std::move(list));
					}
					unusedTimelines.push_back(std::move(iter->timelineSema));
					iter = unfinishedSubmits.erase(iter);
				}
				else {
					++iter;
				}
			}
		}

		void Queue::present(SwapchainImage&& img, SignalHandle& waitOnSignal) {
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
			vkQueuePresentKHR(queue, &presentInfo);
		}

		void Queue::waitForFlush() {
			for (auto& pending : unfinishedSubmits) {
				VkSemaphoreWaitInfo semaphoreWaitInfo{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
					.pNext = nullptr,
					.flags = 0,
					.semaphoreCount = 1,
					.pSemaphores = &pending.timelineSema.timelineSema,
					.pValues = &pending.finishCounter,
				};
				vkWaitSemaphores(device, &semaphoreWaitInfo, UINT64_MAX);
			}
		}
	}
}