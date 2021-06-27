#pragma once

#include "../platform/Window.hpp"

#include "Camera.hpp"
#include "Vulkan.hpp"
#include "ImageManager.hpp"
#include "Renderer2d.hpp"

namespace daxa {

	class Renderer {
	public:
		struct Frame {
			vk::UniqueSemaphore presentSema;
			vkh::CommandBufferAllocator cmdAlloc;
		};

		Renderer(OwningMutex<Window>* winMtx) : winMtx{ winMtx } {
			auto win = winMtx->lock();
			this->framesInFlight = win->imagesInFlight;

			vkh::CommandBufferAllocator cmda{ device,{} };
			auto cmd = cmda.getElement();

			vk::UniqueFence fence = device.createFenceUnique(vkh::makeDefaultFenceCI());

			auto size = win->getSize();

			mainImage = createImage(device, cmd, *fence, size[0], size[1], nullptr);
			depthImage = makeImage(
				vk::ImageCreateInfo{
					.imageType = vk::ImageType::e2D,
					.format = DEPTH_IMAGE_FORMAT,
					.extent = vk::Extent3D{.width = size[0],.height = size[1],.depth = 1},
					.mipLevels = 1,
					.arrayLayers = 1,
					.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
				},
				vk::ImageViewCreateInfo{
				.image = depthImage.image,
				.viewType = vk::ImageViewType::e2D,
				.format = DEPTH_IMAGE_FORMAT,
				.subresourceRange = vk::ImageSubresourceRange {
					.aspectMask = vk::ImageAspectFlagBits::eDepth,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
					}
				}
			);

			for (u32 i = 0; i < framesInFlight; ++i) {
				frames.push_back(Frame{
					.presentSema = device.createSemaphoreUnique({}),
					.cmdAlloc = vkh::CommandBufferAllocator{device, {}}
				});
			}
		}

		void draw() {
			auto win = winMtx->lock();
			u32 imageIndex{ 0 };
			auto& frame = frames[frameIndex];
			device.acquireNextImageKHR(win->swapchain, 1, frame.presentSema.get(), nullptr, &imageIndex);

			frame.cmdAlloc.flush();


			auto cmd = frame.cmdAlloc.get();

			frameIndex = (frameIndex + 1) % framesInFlight;
		}

		OwningMutex<FPSCamera> camera;
		OwningMutex<ImageManager> imgMtx;
		Renderer2d render2d{ device, &imgMtx };
	private:
		vk::Device device{ VulkanGlobals::device };
		OwningMutex<Window>* winMtx{ nullptr };

		u32 framesInFlight{ 2 };
		u32 frameIndex{ 0 };
		std::vector<Frame> frames;


		Image mainImage;
		vk::Format DEPTH_IMAGE_FORMAT{ vk::Format::eD32Sfloat };
		Image depthImage;
	};
}
