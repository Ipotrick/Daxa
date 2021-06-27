#pragma once

#include <array>

#include "../DaxaCore.hpp"

#include "../threading/Jobs.hpp"

#include "Rendering.hpp"
#include "ImageManager.hpp"
#include "UISprite.hpp"

namespace daxa {
	struct Renderer2dFrame {
		vk::UniqueFramebuffer framebuffer;
		vkh::CommandBufferAllocator cmdAlloc;
		vk::UniqueFence fence;
		std::vector<UISprite> spriteInstances;
		std::vector<ReadOnlyLock<Image>> imageLocks;
	};

	class Renderer2dMainJob : IJob {
	public:
		virtual bool execute() override {

			return false;
		}
	};

	class Renderer2d {
	public:
		Renderer2d(vk::Device device, daxa::OwningMutex<ImageManager>* img)
			: device{ device }
			, imgManager{img} 
			, descSetLayoutCache{ device }
		{
			for (int i = 0; i < 2; ++i) {
				frames[i].fence = device.createFenceUnique(vkh::makeDefaultFenceCI());
				frames[i].cmdAlloc = vkh::CommandBufferAllocator(device, {});
			}
		}

		void wait() {
			VulkanGlobals::device.waitForFences( *frames[frameIndex].fence, true, 10000000000);
			if (Jobs::exists(renderJobHandle)) {
				Jobs::wait(renderJobHandle);
			}
		}

		void drawTo(Image& target) {
			wait();

			std::array<vk::ImageView, 2> attachments{ target.view.get(), depthBuffer.view.get() };

			vk::FramebufferCreateInfo framebufferCI{
				.renderPass = renderpass.get(),
				.attachmentCount = (u32)attachments.size(),
				.pAttachments = attachments.data(),
				.width = target.info.extent.width,
				.height = target.info.extent.height,
				.layers = 1,
			};

			frames[frameIndex].framebuffer = device.createFramebufferUnique(framebufferCI);

			frameIndex += 1;
		}

		uint32_t getFrameIndex() const { return frameIndex; }

		std::vector<UISprite>& getSprites() { return frames[frameIndex].spriteInstances; }
		const std::vector<UISprite>& getSprites() const { return frames[frameIndex].spriteInstances; }

		bool bEnableInstancing{ false };
	private:
		vk::Device device;
		daxa::OwningMutex<vkh::DescriptorSetLayoutCache> descSetLayoutCache;
		std::array<Renderer2dFrame, 2> frames;
		std::atomic_uint32_t frameIndex{ 0 };
		daxa::Image depthBuffer;
		vk::UniqueRenderPass renderpass;

		Jobs::Handle renderJobHandle;

		daxa::OwningMutex<ImageManager>* imgManager{ nullptr };

	};
}
