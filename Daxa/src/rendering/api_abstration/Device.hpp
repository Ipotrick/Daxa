#pragma once

#include <deque>
#include <memory>
#include <span>

#include <vulkan/vulkan.h>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/VkBootstrap.hpp"

#include "../../DaxaCore.hpp"

#include "CommandList.hpp"
#include "Image.hpp"
#include "Buffer.hpp"
#include "ShaderModule.hpp"
#include "Pipeline.hpp"
#include "SwapchainImage.hpp"
#include "DescriptorSetLayoutCache.hpp"

namespace daxa {
	namespace gpu {

		class Device {
		public:
			Device() = default;
			Device(Device const& other) = delete;
			~Device();

			static std::shared_ptr<Device> createNewDevice();

			static std::shared_ptr<vkb::Instance> getInstance();

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
			 * \return returns an empty CommandList.
			 */
			CommandList getEmptyCommandList();

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
			std::optional<ShaderModuleHandle> tryCreateShderModuleFromGLSL(std::filesystem::path const& pathToGlsl, VkShaderStageFlagBits stage, std::string const& entryPoint = "main");

			/**
			 * Creates a graphics pipeline, wich is specified by the pipeline builder.
			 * 
			 * \param pipelineBuilder specifies how the pipeline is layed out.
			 * \return a handle to a new graphics pipeline created from the pipeline builder.
			 */
			GraphicsPipelineHandle createGraphicsPipeline(GraphicsPipelineBuilder& pipelineBuilder);

			struct SubmitInfo {
				std::span<VkSemaphore>	waitOnSemaphores;
				std::span<VkSemaphore>	signalSemaphores;
			};

			/**
			 * Submits a CommandList to be executed on the GPU.
			 *
			 * It is guaranteed that all ressouces used in the cmdList and the cmdList itself live until after the gpu has finished executing it.
			 *
			 * \param cmdList holds commands to be executed.
			 * \param submitInfo defines the syncronization on gpu of this submit.
			 */
			void submit(CommandList&& cmdList, SubmitInfo const& submitInfo);

			/**
			 * Submit CommandLists to be executed on the GPU.
			 * Per default the submits are synced to wait on each other based on submission ordering.
			 *
			 * It is guaranteed that all ressouces used in the cmdList and the cmdList itself live until after the gpu has finished executing it.
			 * 
			 * \param cmdLists a vector containing the command lists. This vector will be emptied.
			 * \param submitInfo defines the syncronization on gpu of this submit.
			 */
			void submit(std::vector<CommandList>& cmdLists, SubmitInfo const& submitInfo);

			void present(SwapchainImage const& sImage, std::span<VkSemaphore> waitOn);

			/**
			 * Marks the beginning of the next frame of execution.
			 * If the GPU stalls, this will block until a frame context is available again.
			 * As soon as a new frame context is taken, all framed ressources are freed and or reused for the next frame.
			 */
			void nextFrameContext();

			/**
			 * Waits for the device to complete all submitted operations and idle.
			 */
			void waitIdle();

			operator VkDevice() {
				return getVkDevice();
			}

			const VkPhysicalDevice& getVkPhysicalDevice() { return physicalDevice; }
			const VkDevice& getVkDevice() { return device; }
			const VmaAllocator& getVma() { return allocator; }
			const VkQueue& getVkGraphicsQueue() { return graphicsQ; }
			const u32& getVkGraphicsQueueFamilyIndex() { return graphicsQFamilyIndex; }

		private:
			void initFrameContexts();
			VkSemaphore getNextSemaphore();
			VkFence getNextFence();
			CommandList getNextCommandList();

			std::vector<CommandList> unusedCommandLists;
			std::vector<VkSemaphore> unusedSemaphores;
			std::vector<VkFence> unusedFences;

			struct FrameContext {
				std::vector<CommandList> usedCommandLists;
				std::vector<VkSemaphore> usedSemaphores;
				std::vector<VkFence> usedFences;
				std::vector<ImageHandle> usedImages;
				std::vector<BufferHandle> usedBuffers;
			};

			// VK_KHR_dynamic_rendering:
			void (*vkCmdBeginRenderingKHR)(VkCommandBuffer, const VkRenderingInfoKHR*);
			void (*vkCmdEndRenderingKHR)(VkCommandBuffer);

			std::vector<VkCommandBuffer> submitCommandBufferBuffer;

			std::deque<std::unique_ptr<FrameContext>> frameContexts;

			std::optional<DescriptorSetLayoutCache> descriptorLayoutCache;
			VkPhysicalDevice physicalDevice;
			VkDevice device;
			VmaAllocator allocator;
			VkQueue graphicsQ;
			u32 graphicsQFamilyIndex;
		};
	}
}