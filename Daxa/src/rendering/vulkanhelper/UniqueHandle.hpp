#pragma once

#include "../Vulkan.hpp"

namespace daxa {
	namespace vkh {
		template<typename T>
		void vulkanDestroyFunction(VkDevice device, T handle) { static_assert(false); }

		template<> inline void vulkanDestroyFunction<VkFence>(VkDevice device, VkFence handle) { vkDestroyFence(device, handle, nullptr); }
		template<> inline void vulkanDestroyFunction<VkSemaphore>(VkDevice device, VkSemaphore handle) { vkDestroySemaphore(device, handle, nullptr); }
		template<> inline void vulkanDestroyFunction<VkPipeline>(VkDevice device, VkPipeline handle) { vkDestroyPipeline(device, handle, nullptr); }
		template<> inline void vulkanDestroyFunction<VkRenderPass>(VkDevice device, VkRenderPass handle) { vkDestroyRenderPass(device, handle, nullptr); }
		template<> inline void vulkanDestroyFunction<VkFramebuffer>(VkDevice device, VkFramebuffer handle) { vkDestroyFramebuffer(device, handle, nullptr); }

		template<typename T>
		class UniqueHandle {
		public:
			UniqueHandle() = default;

			UniqueHandle(T handle, VkDevice device = vkh::mainDevice) :
				device{ device }, handle{ handle }
			{
			}

			UniqueHandle(UniqueHandle&& other)
			{
				this->device = other.device;
				this->handle = other.handle;
				other.device = VK_NULL_HANDLE;
				other.handle = VK_NULL_HANDLE;
			}

			UniqueHandle& operator=(UniqueHandle&& other)
			{
				assert(!valid());
				if (&other == this) return *this;
				else return *::new(this) UniqueHandle(std::move(other));
			}

			~UniqueHandle()
			{
				if (valid()) {
					reset();
				}
			}

			T& operator*() { return handle; }

			T* operator->() { return &handle; }

			void reset()
			{
				assert(valid());
				vulkanDestroyFunction<T>(device, handle);
			}

			bool valid() const { return handle != VK_NULL_HANDLE && device != VK_NULL_HANDLE; }

			operator bool() const { return valid(); }

		private:
			VkDevice device{ VK_NULL_HANDLE };
			T handle{ VK_NULL_HANDLE };
		};
	}
}
