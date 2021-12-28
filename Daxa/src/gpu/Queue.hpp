#pragma once

#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <mutex>
#include <vector>

#include <vulkan/vulkan.h>

#include "CommandList.hpp"
#include "TimelineSemaphore.hpp"
#include "Swapchain.hpp"
#include "Signal.hpp"

namespace daxa {
	namespace gpu {
		
		struct SubmitInfo {
			std::vector<CommandListHandle>						commandLists 		= {};		// TODO REPLACE THIS VECTOR WITH HEAPLESS VERSION
			std::span<std::tuple<TimelineSemaphoreHandle, u64>>	waitOnTimelines 	= {};
			std::span<std::tuple<TimelineSemaphoreHandle, u64>>	signalTimelines 	= {};
			std::span<SignalHandle>								waitOnSignals 		= {};
			std::span<SignalHandle>								signalOnCompletion 	= {};
		};

		class Queue {
		public:
			Queue(VkDevice device, VkQueue queue, u32 batchCount = 0);
			Queue() 									= default;
			Queue(Queue const&) 						= delete;
			Queue& operator=(Queue const&) 				= delete;
			Queue(Queue&&) noexcept						= delete;
			Queue& operator=(Queue&&) noexcept			= delete;
			~Queue();

			/**
			 * Submit CommandLists to be executed on the GPU.
			 * Per default the submits are NOT synced to wait on each other based on submission ordering.
			 *
			 * It is guaranteed that all ressouces used in the cmdLists and the cmdLists themselfes are kept alive until after the gpu has finished executing it.
			 *
			 * \param submitInfo contains all information about the submit.
			 * \return a fence handle that can be used to check if the execution is complete or waited upon completion.
			 */
			void submit(SubmitInfo submitInfo);
			
			void submitBlocking(SubmitInfo submitInfo);

			void present(SwapchainImage&& img, SignalHandle& waitOnSignal);

			void checkForFinishedSubmits();

			void nextBatch();

			void waitIdle();
		private: 
			friend class Device;

			VkDevice device = {};
			VkQueue queue = {};

			TimelineSemaphoreHandle getNextTimeline();

			struct PendingSubmit {
				std::vector<CommandListHandle> cmdLists;
				TimelineSemaphoreHandle timelineSema;
				u64 finishCounter = 0;
			};
			std::deque<std::vector<PendingSubmit>> batches;
			bool bWaitForBatchesToComplete = false;

			std::vector<TimelineSemaphoreHandle> unusedTimelines = {};

			// reused temporary buffers:
			std::vector<VkCommandBuffer> submitCommandBufferBuffer = {};
			std::vector<VkSemaphore> submitSemaphoreWaitOnBuffer = {};
			std::vector<VkSemaphore> submitSemaphoreSignalBuffer = {};
			std::vector<u64> submitSemaphoreWaitOnValueBuffer = {};
			std::vector<u64> submitSemaphoreSignalValueBuffer = {};
		};

		class QueueHandle {
		public:
			QueueHandle(std::shared_ptr<Queue> queue) 
				: queue{ std::move(queue) }
			{}
			QueueHandle() = default;

			Queue const& operator*() const { return *queue; }
			Queue& operator*() { return *queue; }
			Queue const* operator->() const { return queue.get(); }
			Queue* operator->() { return queue.get(); }

			size_t getRefCount() const { return queue.use_count(); }

			operator bool() const { return queue.operator bool(); }
			bool operator!() const { return !queue; }
			bool valid() const { return *this; }
		private:
			std::shared_ptr<Queue> queue = {};
		};
	}
}