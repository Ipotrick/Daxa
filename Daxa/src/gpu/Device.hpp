#pragma once

#include "../DaxaCore.hpp"

#include <deque>
#include <memory>
#include <span>

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.hpp>
#include <VkBootstrap.h>

#include "CommandList.hpp"
#include "Image.hpp"
#include "Buffer.hpp"
#include "ShaderModule.hpp"
#include "Pipeline.hpp"
#include "SwapchainImage.hpp"
#include "TimelineSemaphore.hpp"
#include "RenderWindow.hpp"
#include "Signal.hpp"
#include "StagingBufferPool.hpp"
#include "Queue.hpp"

namespace daxa {
	namespace gpu {

		class Device {
		public:
			Device(Device const& other)				= delete;
			Device& operator=(Device const&)		= delete;
			Device(Device&&) noexcept;
			Device& operator=(Device&&) noexcept;
			~Device();

			Queue createQueue();

			static Device create();

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

			TimelineSemaphore createTimelineSemaphore();

			SignalHandle createSignal();

			RenderWindow createRenderWindow(VkSurfaceKHR surface, u32 width, u32 height, VkPresentModeKHR presentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR);

			BindingSetDescription const* createBindingSetDescription(std::span<VkDescriptorSetLayoutBinding> bindings);

			/**
			 * \param glslSource a string with valid glsl source code.
			 * \param stage the shader stage, can be any of the VkShaderStageFlagBits.
			 * \param name a name for the shader stage, only used for debugging.
			 * \return newly created ShaderModule from the given source IF source is valid, or a nullopt if the source or path is invalid.
			 */
			std::optional<ShaderModuleHandle> tryCreateShderModuleFromGLSL(std::string const& glslSource, VkShaderStageFlagBits stage, std::string const& entryPoint = "main");

			/**
			 * \param pathToGlsl a path to a file that contains valid glsl source code.
			 * \param stage the shader stage, can be any of the VkShaderStageFlagBits.
			 * \param name a name for the shader stage, only used for debugging.
			 * \return newly created ShaderModule from the given source IF source path is valid, or a nullopt if the source or path is invalid.
			 */
			std::optional<ShaderModuleHandle> tryCreateShderModuleFromFile(std::filesystem::path const& pathToGlsl, VkShaderStageFlagBits stage, std::string const& entryPoint = "main");

			/**
			 * Creates a graphics pipeline, wich is specified by the pipeline builder.
			 *
			 * \param pipelineBuilder specifies how the pipeline is layed out.
			 * \return a handle to a new graphics pipeline created from the pipeline builder.
			 */
			GraphicsPipelineHandle createGraphicsPipeline(GraphicsPipelineBuilder& pipelineBuilder);

			BindingSetAllocator createBindingSetAllocator(BindingSetDescription const* setDescription, size_t setPerPool = 64);

			/**
			 * \return returns an empty CommandList.
			 */
			CommandList getEmptyCommandList();

			/**
			 * Waits for the device to complete all submitted operations and the gpu to idle.
			 */
			void waitIdle();

			const VkPhysicalDevice& getVkPhysicalDevice() const { return physicalDevice; }
			const VkDevice& getVkDevice() const { return device; }
			const VmaAllocator& getVma() const { return allocator; }
			//const VkQueue& getVkGraphicsQueue() const { return graphicsQ; }
			const u32& getVkGraphicsQueueFamilyIndex() const { return graphicsQFamilyIndex; }
		private:
			friend class Instance;

			Device(vkb::Instance&);

			CommandList getNextCommandList();

			VkInstance instance = VK_NULL_HANDLE;
			VkDevice device = VK_NULL_HANDLE;
			vkb::Device vkbDevice = {};
			VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
			VmaAllocator allocator = VK_NULL_HANDLE;
			u32 graphicsQFamilyIndex = 0;

			std::shared_ptr<CommandListRecyclingSharedData> cmdListRecyclingSharedData = {};
			std::vector<CommandList> unusedCommandLists;

			// VK_KHR_dynamic_rendering:
			void (*vkCmdBeginRenderingKHR)(VkCommandBuffer, const VkRenderingInfoKHR*) = nullptr;
			void (*vkCmdEndRenderingKHR)(VkCommandBuffer) = nullptr;
			// Synchronization2KHR:
			void (*vkCmdPipelineBarrier2KHR)(VkCommandBuffer, VkDependencyInfoKHR const*) = nullptr;

			std::shared_ptr<StagingBufferPool> stagingBufferPool = {};

			std::unique_ptr<BindingSetDescriptionCache> bindingSetDescriptionCache = {};
		};
	}
}