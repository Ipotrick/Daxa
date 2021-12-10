#pragma once

#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <mutex>
#include <vector>

#include <vulkan/vulkan.h>

#include "CommandList.hpp"
#include "TimelineSemaphore.hpp"
#include "RenderWindow.hpp"
#include "Signal.hpp"

namespace daxa {
	namespace gpu {
		
		struct SubmitInfo {
			std::vector<CommandListHandle>					commandLists;		// TODO REPLACE THIS VECTOR WITH HEAPLESS VERSION
			std::span<std::tuple<TimelineSemaphore*, u64>>	waitOnTimelines;
			std::span<std::tuple<TimelineSemaphore*, u64>>	signalTimelines;
			std::span<SignalHandle>							waitOnSignals;
			std::span<SignalHandle>							signalOnCompletion;
		};

		class Queue {
		public:
			Queue() = default;
			Queue(Queue const&) = delete;
			Queue& operator=(Queue const&) = delete;
			Queue(Queue&&) noexcept;
			Queue& operator=(Queue&&) noexcept;
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
			void submit(SubmitInfo& submitInfo);

			void present(SwapchainImage&& img, SignalHandle& waitOnSignal);

			void checkForFinishedSubmits();

			void waitForFlush();
		private: 
			friend class Device;

			Queue(VkDevice device, VkQueue queue);

			VkDevice device = {};
			VkQueue queue = {};

			TimelineSemaphore getNextTimeline();

			struct PendingSubmit {
				std::vector<CommandListHandle> cmdLists;
				TimelineSemaphore timelineSema;
				u64 finishCounter = 0;
			};
			std::vector<PendingSubmit> unfinishedSubmits = {};

			std::vector<TimelineSemaphore> unusedTimelines = {};

			// reused temporary buffers:
			std::vector<VkCommandBuffer> submitCommandBufferBuffer = {};
			std::vector<VkSemaphore> submitSemaphoreWaitOnBuffer = {};
			std::vector<VkSemaphore> submitSemaphoreSignalBuffer = {};
			std::vector<u64> submitSemaphoreWaitOnValueBuffer = {};
			std::vector<u64> submitSemaphoreSignalValueBuffer = {};
		};
	}
}