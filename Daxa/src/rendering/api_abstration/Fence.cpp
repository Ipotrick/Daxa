#include "Fence.hpp"

#include "common.hpp"

namespace daxa {
	namespace gpu {
		DAXA_DEFINE_TRIVIAL_MOVE(Fence)

		Fence::Fence(VkDevice device) 
			: device{ device }
		{
			VkFenceCreateInfo fenceCI{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.pNext = nullptr,
			};
			vkCreateFence(device, &fenceCI, nullptr, &fence);
		}

		Fence::~Fence() {
			if (device && fence) {
				vkDestroyFence(device, fence, nullptr);
				std::memset(this, 0, sizeof(Fence));
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

		FenceHandle::~FenceHandle() {
			if (fence) {
				if (fence.use_count() == 1) {
					recyclingVec.lock()->push_back(std::move(*fence));
				}
			}
		}

		FenceHandle::FenceHandle(Fence&& fence, std::weak_ptr<std::vector<Fence>>&& recyclingVec)
			: fence{ std::make_shared<Fence>(std::move(fence)) }
			, recyclingVec{ recyclingVec }
		{ }

		FenceHandle::operator bool() const {
			return isValid();
		}
	}
}