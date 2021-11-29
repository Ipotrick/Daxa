#pragma once

#include <unordered_map> 
#include <memory> 

#include "../platform/Window.hpp"

#include "Camera.hpp"

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
		std::unordered_map<std::string_view, vk::UniqueFence> fences;
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
				frameResc[i].semaphores["aquireSwapchainImage"] = device->getVkDevice().createSemaphoreUnique({});
			}
		}

		void draw() {
			auto& frame = frameResc.front();

			auto swapchainImage = renderWindow.getNextImage(*frame.semaphores["aquireSwapchainImage"]);

			auto cmdList = device->getEmptyCommandList();

			cmdList.begin();

			cmdList.changeImageLayout(swapchainImage.getImageHandle(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			std::array colorAttachments{
				gpu::RenderAttachmentInfo{
					.image = swapchainImage.getImageHandle(),
					.clearValue = VkClearValue{.color = VkClearColorValue{.float32 = { 0.4f, 0.5f, 0.7f, 1.0f } } },
				}
			};
			gpu::BeginRenderingInfo renderInfo;
			renderInfo.colorAttachmentCount = (u32)colorAttachments.size();
			renderInfo.colorAttachments = colorAttachments.data();
			cmdList.beginRendering(renderInfo);

			cmdList.endRendering();

			cmdList.changeImageLayout(swapchainImage.getImageHandle(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

			cmdList.end();

			device->submit(std::move(cmdList), {
				.waitOnSemaphores = { *frame.semaphores["aquireSwapchainImage"] },
				.signalSemaphores = { *frame.semaphores["render"] },
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
