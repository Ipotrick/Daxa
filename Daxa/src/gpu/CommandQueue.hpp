#pragma once

#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <mutex>
#include <vector>

#include <vulkan/vulkan.h>

#include "Handle.hpp"
#include "CommandList.hpp"
#include "TimelineSemaphore.hpp"
#include "Swapchain.hpp"
#include "Signal.hpp"

namespace daxa {
	struct CommandQueueCreateInfo {
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

	struct CommandListFetchInfo {
		char const* debugName = {};
	};

	class CommandQueue {
	public:
		CommandQueue(
			std::shared_ptr<DeviceBackend> deviceBackend, 
			VkQueue queue, 
			u32 queueFamilyIndex, 
			std::shared_ptr<StagingBufferPool> uploadStagingBufferPool, 
			std::shared_ptr<StagingBufferPool> downloadStagingBufferPool, 
			CommandQueueCreateInfo const& ci
		);
		CommandQueue() 										= default;
		CommandQueue(CommandQueue const&) 					= delete;
		CommandQueue& operator=(CommandQueue const&) 		= delete;
		CommandQueue(CommandQueue&&) noexcept				= delete;
		CommandQueue& operator=(CommandQueue&&) noexcept	= delete;
		~CommandQueue();

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

		CommandListHandle getCommandList(CommandListFetchInfo const& fi);

		std::string const& getDebugName() const { return debugName; }
	private: 
		friend class Device;

		TimelineSemaphoreHandle getNextTimeline();
		CommandListHandle getNextCommandList();

		struct PendingSubmit {
			std::vector<CommandListHandle> 	cmdLists		= {};
			TimelineSemaphoreHandle 		timelineSema	= {};
			u64 							finishCounter 	= 0;
		};

		std::shared_ptr<DeviceBackend> 					deviceBackend 						= {};
		VkQueue 										queue 								= {};
		u32												queueFamilyIndex					= {};
		std::deque<std::vector<PendingSubmit>> 			batches								= {};
		bool 											bWaitForBatchesToComplete 			= false;
		std::vector<TimelineSemaphoreHandle> 			unusedTimelines 					= {};
		std::shared_ptr<CommandListRecyclingSharedData> cmdListRecyclingSharedData 			= std::make_shared<CommandListRecyclingSharedData>();
		std::shared_ptr<StagingBufferPool> 				uploadStagingBufferPool 			= {};
		std::vector<CommandListHandle> 					unusedCommandLists					= {};
		std::string 									debugName 							= {};
		// reused temporary buffers:		
		std::vector<VkCommandBuffer> 					submitCommandBufferBuffer 			= {};
		std::vector<VkSemaphore> 						submitSemaphoreWaitOnBuffer 		= {};
		std::vector<VkSemaphore> 						submitSemaphoreSignalBuffer 		= {};
		std::vector<u64> 								submitSemaphoreWaitOnValueBuffer 	= {};
		std::vector<u64> 								submitSemaphoreSignalValueBuffer 	= {};
	};

	class CommandQueueHandle : public SharedHandle<CommandQueue>{};
}