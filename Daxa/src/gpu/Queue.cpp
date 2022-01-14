#include "Queue.hpp"
#include "Instance.hpp"

namespace daxa {
	namespace gpu {

		Queue::Queue(VkDevice device, VkQueue queue, QueueCreateInfo const& ci)
			: device{ device }
			, queue{ queue }
			, bWaitForBatchesToComplete{ ci.batchCount > 0 }
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
					instance->pfnSetDebugUtilsObjectNameEXT(this->device, &imageNameInfo);
				}
			}
		}

		Queue::~Queue() {
			waitIdle();
		}

		void Queue::submit(SubmitInfo si) {
			for (auto& cmdList : si.commandLists) {
				DAXA_ASSERT_M(cmdList->finalized, "can only submit finalized command lists");
				for (auto& sbuffer : cmdList->usedStagingBuffers) {
					DAXA_ASSERT_M(!sbuffer.buffer->isMemoryMapped(), "can not submit command list. Some Buffers used in the command list have mapped memory, all memory to used buffers need to be unmapped before a submit.");
				}

				submitCommandBufferBuffer.push_back(cmdList->cmd);
				cmdList->usesOnGPU += 1;
			
				for (auto& set : cmdList->usedSets) {
					set->usesOnGPU += 1;

					for (auto& handle : set->handles) {
						if (auto* buffer = std::get_if<BufferHandle>(&handle)) {
							(**buffer).usesOnGPU += 1;
						}
					}
				}
				for (auto& buffer : cmdList->usedBuffers) {
					DAXA_ASSERT_M(!buffer->isMemoryMapped(), "can not submit command list. Some Buffers used in the command list have mapped memory, all memory to used buffers need to be unmapped before a submit.");
					buffer->usesOnGPU += 1;
				}
			}

			for (auto& signal : si.waitOnSignals) {
				this->submitSemaphoreWaitOnBuffer.push_back(signal->getVkSemaphore());
				this->submitSemaphoreWaitOnValueBuffer.push_back(0);
			}
			for (auto [timelineSema, waitValue] : si.waitOnTimelines) {
				this->submitSemaphoreWaitOnBuffer.push_back(timelineSema->getVkSemaphore());
				this->submitSemaphoreWaitOnValueBuffer.push_back(waitValue);
			}
			for (auto signal : si.signalOnCompletion) {
				this->submitSemaphoreSignalBuffer.push_back(signal->getVkSemaphore());
				this->submitSemaphoreSignalValueBuffer.push_back(0);
			}
			for (auto [timelineSema, signalValue] : si.signalTimelines) {
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
			vkQueueSubmit(queue, 1, &submitInfo, nullptr);

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

		TimelineSemaphoreHandle Queue::getNextTimeline() {
			if (unusedTimelines.empty()) {
				unusedTimelines.push_back(TimelineSemaphoreHandle{ std::make_shared<TimelineSemaphore>(device, TimelineSemaphoreCreateInfo{.debugName = "queue timline"}) });
			}
			auto timeline = std::move(unusedTimelines.back());
			unusedTimelines.pop_back();
			return timeline;
		}

		void Queue::submitBlocking(SubmitInfo submitInfo) {
			submit(submitInfo);
			batches[0].back().timelineSema->wait(batches[0].back().finishCounter);
		}

		void Queue::checkForFinishedSubmits() {
			for (auto& batch : batches) {
				for (auto iter = batch.begin(); iter != batch.end();) {
					if (iter->timelineSema->getCounter() >= iter->finishCounter) {
						while (!iter->cmdLists.empty()) {
							auto list = std::move(iter->cmdLists.back());
							iter->cmdLists.pop_back();
							for (auto& buffer : list->usedBuffers) {
								buffer.buffer->usesOnGPU -= 1;
							}
							for (auto& set : list->usedSets) {
								set->usesOnGPU -= 1;

								for (auto& handle : set->handles) {
									if (auto* buffer = std::get_if<BufferHandle>(&handle)) {
										(**buffer).usesOnGPU -= 1;
									}
								}
							}
							list->usesOnGPU -= 1;
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

		void Queue::waitIdle() {
			for (auto& batch : batches) {
				for (auto& pending : batch) {
					pending.timelineSema->wait(pending.finishCounter);
				}
			}
		}

		void Queue::nextBatch() {
			auto back = std::move(batches.back());
			batches.pop_back();
			batches.push_front(std::move(back));
			if (bWaitForBatchesToComplete) {
				for (auto& pending : batches[0]) {
					pending.timelineSema->wait(pending.finishCounter);
				}
			}
		}
	}
}