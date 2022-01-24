#pragma once

#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <mutex>
#include <vector>

#include <vulkan/vulkan.h>

#include "Handle.hpp"
#include "DeviceBackend.hpp"
#include "CommandList.hpp"
#include "TimelineSemaphore.hpp"
#include "Swapchain.hpp"
#include "Signal.hpp"

namespace daxa {
	namespace gpu {

		struct QueueCreateInfo {
			u32 		batchCount	= 0;
			char const* debugName 	= {};
		};
		
		struct SubmitInfo {
			std::vector<CommandListHandle>						commandLists 		= {};		// TODO REPLACE THIS VECTOR WITH HEAPLESS VERSION
			std::span<std::tuple<TimelineSemaphoreHandle, u64>>	waitOnTimelines 	= {};
			std::span<std::tuple<TimelineSemaphoreHandle, u64>>	signalTimelines 	= {};
			std::span<SignalHandle>								waitOnSignals 		= {};
			std::span<SignalHandle>								signalOnCompletion 	= {};
		};

		class Queue {
		public:
			Queue(std::shared_ptr<DeviceBackend> deviceBackend, VkQueue queue, QueueCreateInfo const& ci);
			Queue() 							= default;
			Queue(Queue const&) 				= delete;
			Queue& operator=(Queue const&) 		= delete;
			Queue(Queue&&) noexcept				= delete;
			Queue& operator=(Queue&&) noexcept	= delete;
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

			std::string const& getDebugName() const { return debugName; }
		private: 
			friend class Device;

			TimelineSemaphoreHandle getNextTimeline();

			struct PendingSubmit {
				std::vector<CommandListHandle> 	cmdLists		= {};
				TimelineSemaphoreHandle 		timelineSema	= {};
			u64 								finishCounter 	= 0;
			};

			std::shared_ptr<DeviceBackend> 			deviceBackend 						= {};
			VkQueue 								queue 								= {};
			std::deque<std::vector<PendingSubmit>> 	batches								= {};
			bool 									bWaitForBatchesToComplete 			= false;
			std::vector<TimelineSemaphoreHandle> 	unusedTimelines 					= {};
			// reused temporary buffers:
			std::vector<VkCommandBuffer> 			submitCommandBufferBuffer 			= {};
			std::vector<VkSemaphore> 				submitSemaphoreWaitOnBuffer 		= {};
			std::vector<VkSemaphore> 				submitSemaphoreSignalBuffer 		= {};
			std::vector<u64> 						submitSemaphoreWaitOnValueBuffer 	= {};
			std::vector<u64> 						submitSemaphoreSignalValueBuffer 	= {};
			std::string 							debugName 							= {};
		};

		class QueueHandle : public SharedHandle<Queue>{};
	}
}