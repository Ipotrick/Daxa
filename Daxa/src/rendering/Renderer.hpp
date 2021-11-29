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
		std::unordered_map<std::string_view, VkSemaphore> semaphores;
		std::unordered_map<std::string_view, VkFence> fences;
	};

	class Renderer {
	public:
		Renderer(std::shared_ptr<Window> win);
		~Renderer() {
			for (int i = 0; i < 3; i++) {
				nextFrameContext();			// wait for all frames in flight to complete
			}
			for (auto& frame : frameResc) {
				for (auto& [name, sema] : frame.semaphores) {
					vkDestroySemaphore(device->getVkDevice(), sema, nullptr);
				}
				for (auto& [name, fence] : frame.fences) {
					vkDestroyFence(device->getVkDevice(), fence, nullptr);
				}
			}
			frameResc.clear();
			persResc.reset();
		}

		void init() {
			for (int i = 0; i < 3; i++) {
				frameResc[i].semaphores["render"] = device->getVkDevice().createSemaphore({});
				frameResc[i].semaphores["aquireSwapchainImage"] = device->getVkDevice().createSemaphore({});
			}
		}

		void draw(float deltaTime) {
			auto& frame = frameResc.front();

			auto swapchainImage = renderWindow.getNextImage(frame.semaphores["aquireSwapchainImage"]);

			auto cmdList = device->getEmptyCommandList();

			cmdList.begin();

			cmdList.changeImageLayout(swapchainImage.getImageHandle(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			double intpart;
			totalElapsedTime += deltaTime;
			float r = std::cos(totalElapsedTime* 0.21313) * 0.3f + 0.5f;
			float g = std::cos(totalElapsedTime * 0.75454634) * 0.3f + 0.5f;
			float b = std::cos(totalElapsedTime) * 0.3f + 0.5f;
			printf("color: %f, %f, %f\n",r,g,b);

			VkClearValue clear{ .color = VkClearColorValue{.float32 = { r, g, b, 1.0f } } };

			std::array colorAttachments{
				gpu::RenderAttachmentInfo{
					.image = swapchainImage.getImageHandle(),
					.clearValue = clear,
				}
			};
			gpu::BeginRenderingInfo renderInfo;
			renderInfo.colorAttachments = colorAttachments;
			cmdList.beginRendering(renderInfo);

			cmdList.endRendering();

			cmdList.changeImageLayout(swapchainImage.getImageHandle(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

			cmdList.end();

			std::array waitOnSemasSubmit = { frame.semaphores["aquireSwapchainImage"] };
			std::array singalSemasSubmit = { frame.semaphores["render"] };
			device->submit(std::move(cmdList), {
				.waitOnSemaphores = waitOnSemasSubmit,
				.signalSemaphores = singalSemasSubmit,
			});

			std::array waitOnSemasPresent = { frame.semaphores["render"] };
			device->present(swapchainImage, waitOnSemasPresent);

			nextFrameContext();
		}

		void deinit() {  

		}

		FPSCamera camera;
		std::shared_ptr<gpu::Device> device;
		std::optional<PersistentRessources> persResc;
		std::deque<PerFrameRessources> frameResc;
		double totalElapsedTime{ 0.0 };
	private:
		std::shared_ptr<Window> window{ nullptr };
		gpu::RenderWindow renderWindow;
		void nextFrameContext();
	};
}
