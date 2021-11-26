#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

#include "Image.hpp"
#include "Buffer.hpp"

namespace daxa {
	namespace gpu {

		struct BeginRenderingInfo {
			ImageHandle* colorAttachemnts = nullptr;
			size_t colorAttachmentCount = 0;
			ImageHandle* depthStencilAttachment = nullptr;
		};

		class CommandList {
		public:
			~CommandList();

			CommandList(CommandList&& rhs) noexcept;
			CommandList& operator=(CommandList&& rhs) noexcept;

			CommandList(CommandList const& rhs) = delete;
			CommandList& operator=(CommandList const& rhs) = delete;

			void begin();
			void end();

			void beginRendering(BeginRenderingInfo ri);
			void endRendering();

			vk::CommandBuffer getVkCommandBuffer() { return cmd; }
		private:
			friend class Device;

			CommandList();
			void reset();

			bool bIsNotMovedOutOf = true;
			bool empty = true;
			bool bUnfinishedOperationInProgress = false;
			std::vector<ImageHandle> usedImages;
			std::vector<BufferHandle> usedBuffers;
			vk::Device device;
			vk::CommandBuffer cmd;
			vk::CommandPool cmdPool;
		};

	}
}