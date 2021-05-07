#pragma once

#include <array>

#include "../DaxaCore.hpp"

#include "../threading/Jobs.hpp"

#include "Rendering.hpp"
#include "ImageManager.hpp"
#include "UISprite.hpp"

namespace daxa {
	struct Renderer2dFrame {
		vk::Framebuffer framebuffer;
		vkh::CommandBufferAllocator cmdAlloc;
		vk::UniqueFence fence;
		std::vector<UISprite> spriteInstances;
	};

	class Renderer2dMainJob : IJob {
	public:
		virtual bool execute() override {


			return false;
		}
	};

	class Renderer2d {
	public:
		void wait() {
			VulkanContext::device.waitForFences( *frames[frameIndex].fence, true, 10000000000);
			if (Jobs::exists(renderJobHandle)) {
				Jobs::wait(renderJobHandle);
			}
		}

		void draw() {
			wait();

			frameIndex += 1;
		}

		uint32_t getFrameIndex() const { return frameIndex; }

		std::vector<UISprite>& getSprites() { return frames[frameIndex].spriteInstances; }
		const std::vector<UISprite>& getSprites() const { return frames[frameIndex].spriteInstances; }

		daxa::OwningMutex<ImageManager> imgManager;

		bool bEnableInstancing{ false };
	private:
		std::atomic_uint32_t frameIndex{ 0 };
		Jobs::Handle renderJobHandle;

		daxa::Image depthBuffer;
		std::vector<Renderer2dFrame> frames;

		daxa::OwningMutex<vkh::DescriptorSetLayoutCache> descLayoutCache;
	};
}
