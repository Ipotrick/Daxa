#pragma once

#include <unordered_map> 
#include <memory> 

#include "../platform/Window.hpp"

#include "Camera.hpp"
#include "Vulkan.hpp"
#include "ImageManager.hpp"
#include "Image.hpp"

#include "api_abstration/Device.hpp"
#include "api_abstration/RenderWindow.hpp"

namespace daxa {

	struct PersistentRessources {
		std::unordered_map<std::string_view, gpu::GraphicsPipelineHandle> pipelines;
		std::unordered_map<std::string_view, gpu::ImageHandle> images;
		std::unordered_map<std::string_view, gpu::BufferHandle> buffers;
	};

	struct PerFrameRessources {
		std::unordered_map<std::string_view, gpu::ImageHandle> images;
		std::unordered_map<std::string_view, gpu::BufferHandle> buffers;
		std::unordered_map<std::string_view, vk::UniqueSemaphore> semaphores;
	};

	class Renderer {
	public:
		Renderer(std::shared_ptr<Window> win);
		~Renderer() {
			frameResc.clear();
			persResc.reset();
		}

		void init() {
			for (int i = 0; i < 3; i++) {
				frameResc[i].semaphores["render"] = device->getVkDevice().createSemaphoreUnique({});
			}
		}

		void setLayoutOfImage2D(gpu::CommandList& cmd, gpu::Image& image, VkImageLayout dstLayout) {

			VkImageMemoryBarrier imgMemBarr = {};
			imgMemBarr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imgMemBarr.pNext = nullptr;
			imgMemBarr.oldLayout = image.getLayout();
			imgMemBarr.newLayout = dstLayout;
			imgMemBarr.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imgMemBarr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imgMemBarr.image = image.getVkImage();
			imgMemBarr.subresourceRange = VkImageSubresourceRange{
				.aspectMask = image.getVkAspect(),
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			};
			imgMemBarr.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
			imgMemBarr.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

			vkCmdPipelineBarrier(
				cmd.getVkCommandBuffer(),
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_DEPENDENCY_DEVICE_GROUP_BIT,
				0,
				nullptr,
				0,
				nullptr,
				1,
				&imgMemBarr
			);

			image.layout = dstLayout;
		}

		void draw() {
			auto& frame = frameResc.front();

			auto swapchainImage = renderWindow.getNextImage();

			auto cmdList = device->getEmptyCommandList();

			cmdList.begin();

			setLayoutOfImage2D(cmdList, *swapchainImage.getImageHandle(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			std::array colorAttachments{
				gpu::RenderAttachmentInfo{
					.image = swapchainImage.getImageHandle(),
				}
			};
			gpu::BeginRenderingInfo renderInfo;
			renderInfo.colorAttachmentCount = (u32)colorAttachments.size();
			renderInfo.colorAttachments = colorAttachments.data();
			cmdList.beginRendering(renderInfo);

			cmdList.endRendering();

			setLayoutOfImage2D(cmdList, *swapchainImage.getImageHandle(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

			cmdList.end();

			device->submit(std::move(cmdList), {
				.waitOnSemaphores = { swapchainImage.getVkSemaphore() },
				.signalSemaphores = { *frame.semaphores["render"] }
			});

			device->present(swapchainImage, { *frame.semaphores["render"] });

			nextFrameContext();
		}

		void deinit() {  
		}

		FPSCamera camera;
		std::shared_ptr<gpu::Device> device;
		std::optional<PersistentRessources> persResc;
		std::deque<PerFrameRessources> frameResc;
	private:
		std::shared_ptr<Window> window{ nullptr };
		gpu::RenderWindow renderWindow;
		void nextFrameContext();
	};
}
