#pragma once

#include "../../DaxaCore.hpp"

#include <memory>
#include <cassert>
#include <vector>
#include <optional>

#include <vulkan/vulkan.h>

namespace daxa {
	namespace gpu {

		class Fence {
		public:
			Fence() = default;
			Fence(VkDevice device);
			Fence(Fence&&) noexcept;
			Fence& operator=(Fence&&) noexcept;
			~Fence();
		private:
			friend class FenceHandle;
			friend class Device;

			VkDevice device{ VK_NULL_HANDLE };
			VkFence fence{ VK_NULL_HANDLE };
		};

		class FenceHandle {
		public:
			FenceHandle() = default;
			~FenceHandle();
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
			FenceHandle(Fence&&, std::weak_ptr<std::vector<Fence>>&& recyclingVec);

			std::weak_ptr<std::vector<Fence>> recyclingVec = {};
			std::shared_ptr<Fence> fence = {};
		};
	}
}
