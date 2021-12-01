#pragma once

#include "../../DaxaCore.hpp"

#include <memory>
#include <cassert>
#include <vector>

#include <vulkan/vulkan.h>

namespace daxa {
	namespace gpu {

		class Fence {
		public:
			Fence() = default;
			Fence(VkDevice device);
			Fence(Fence&&) noexcept;
			Fence& operator=(Fence&&) noexcept;
			Fence(VkDevice device, std::vector<Fence>* recyclingQueue);
			~Fence();
		private:
			friend class FenceHandle;
			friend class Device;

			void disableRecycling();

			VkDevice device{ VK_NULL_HANDLE };
			VkFence fence{ VK_NULL_HANDLE };
			std::vector<Fence>* recyclingQueue{ nullptr };
		};

		class FenceHandle {
		public:
			FenceHandle() = default;
			~FenceHandle() = default;
			FenceHandle(FenceHandle&&) noexcept = default;
			FenceHandle& operator=(FenceHandle&&) noexcept = default;
			FenceHandle(FenceHandle const& other);
			FenceHandle& operator=(FenceHandle const& other);

			VkResult checkStatus() const;

			VkResult wait(size_t timeout) const;

			bool isValid() const;

			operator bool() const;

			void reset();
		private:
			friend class Device;

			FenceHandle(Fence&&);

			std::shared_ptr<Fence> fence{ nullptr };
		};
	}
}
