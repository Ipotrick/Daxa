#pragma once

#include <deque>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

#include "CommandList.hpp"
#include "Image.hpp"
#include "Buffer.hpp"

namespace gpu {

	class Device {
	public:
		static Device createNewDevice();

		ImageHandle createImage2d(Image2dCreateInfo ci);

		BufferHandle createBuffer(BufferCreateInfo ci);

		BufferHandle createFramedBuffer(BufferCreateInfo ci);


		/**
		 * \return creates new CommandList wich lives until the current frame on the GPU is completed.
		 */
		CommandList createFramedCommandList();

		/**
		 * Submits a CommandList to be executed on the GPU. 
		 * 
		 * \param cmdList holds commands to be executed.
		 */
		void submit(CommandList& cmdList);

		/**
		 * Marks the beginning of the next frame of execution.
		 * If the GPU stalls, this will block until a frame context is available again.
		 * As soon as a new frame context is taken, all framed ressources are freed and or reused for the next frame.
		 */
		void nextFrameContext();

		const vk::PhysicalDevice& getPhysicalDevice() { return physicalDevice; }
		const vk::Device& getDevice() { return device; }
		const VmaAllocator& getVma() { return allocator; }
		const vk::Queue& getGraphicsQ() { return graphicsQ; }
		const u32& getGraphicsQFamilyIndex() { return graphicsQFamilyIndex; }
	private:
		struct FrameContext {
			vk::UniqueCommandPool cmdPool;
			std::vector<vk::Semaphore> unusedSemaphores;
			std::vector<vk::Semaphore> usedSemaphores;
			std::vector<vk::Fence> unusedFences;
			std::vector<vk::Fence> usedFences;
			std::vector<BufferHandle> framedBuffers;
		};

		void initFrameContexts();
		vk::Semaphore getNextSemaphore();
		vk::Fence getNextFence();

		std::deque<std::unique_ptr<FrameContext>> frameContexts;

		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		VmaAllocator allocator;
		vk::Queue graphicsQ;
		u32 graphicsQFamilyIndex;
	};

}
