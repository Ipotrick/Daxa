#include "Fence.hpp"

#include "common.hpp"

namespace daxa {
	namespace gpu {
		DAXA_DEFINE_TRIVIAL_MOVE(Fence)

		Fence::Fence(VkDevice device) {
			this->device = device;
			VkFenceCreateInfo fenceCI{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.pNext = nullptr,
			};
			vkCreateFence(device, &fenceCI, nullptr, &fence);
		}

		Fence::Fence(VkDevice device, std::vector<Fence>* recyclingQueue) : Fence{device} {
			this->recyclingQueue = recyclingQueue;
		}

		Fence::~Fence() {
			if (device && fence) {
				if (recyclingQueue) {
					vkResetFences(device, 1, &fence);
					recyclingQueue->push_back(std::move(*this));
				}
				else {
					vkDestroyFence(device, fence, nullptr);
					std::memset(this, 0, sizeof(Fence));
				}
			}
		}

		FenceHandle::FenceHandle(FenceHandle const& other) {
			this->fence = other.fence;
		}

		FenceHandle& FenceHandle::operator=(FenceHandle const& other) {
			this->fence = other.fence;
			return *this;
		}

		VkResult FenceHandle::checkStatus() const {
			assert(isValid(), "ERROR: tried waiting on an invalid fence handle. Please make sure that your handle is pointing to a ressource!");
			return vkGetFenceStatus(fence->device, fence->fence);
		}

		VkResult FenceHandle::wait(size_t timeout) const {
			return vkWaitForFences(fence->device, 1, &fence->fence, VK_TRUE, timeout);
		}

		bool FenceHandle::isValid() const {
			return fence != nullptr;
		}

		void FenceHandle::reset() {
			fence = nullptr;
		}

		FenceHandle::FenceHandle(Fence&& fence) {
			this->fence = std::make_shared<Fence>(std::move(fence));
		}

		FenceHandle::operator bool() const {
			return isValid();
		}

		void Fence::disableRecycling() {
			this->recyclingQueue = nullptr;
		}
	}
}