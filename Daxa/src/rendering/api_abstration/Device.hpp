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

namespace daxa {
	namespace gpu {

		class Device {
		public:
			static Device createNewDevice();

			/**
			 * \param ci all information defining the 2d image
			 * \return a reference counted image handle of the created image ressource.
			 */
			ImageHandle createImage2d(Image2dCreateInfo ci);

			/**
			 * \param ci all information defining the buffer
			 * \return a reference counted buffer handle of the created buffer ressource.
			 */
			BufferHandle createBuffer(BufferCreateInfo ci);

			/**
			 * Creates a buffer object wich is guaranteed to live for at least one frame context cycle.
			 * \param ci all information defining the buffer
			 * \return a reference counted buffer handle of the created buffer ressource.
			 */
			BufferHandle createFramedBuffer(BufferCreateInfo ci);

			/**
			 * \return returns an empty CommandList.
			 */
			CommandList createCommandList();

			/**
			 * Submits a CommandList to be executed on the GPU.
			 *
			 * \param cmdList holds commands to be executed.
			 */
			void submit(CommandList&& cmdList);

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
				std::vector<CommandList> unusedCommandLists;
				std::vector<CommandList> usedCommandLists;
				std::vector<vk::Semaphore> unusedSemaphores;
				std::vector<vk::Semaphore> usedSemaphores;
				std::vector<vk::Fence> unusedFences;
				std::vector<vk::Fence> usedFences;
				std::vector<BufferHandle> framedBuffers;
				std::vector<ImageHandle> usedImages;
				std::vector<BufferHandle> usedBuffers;
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
}