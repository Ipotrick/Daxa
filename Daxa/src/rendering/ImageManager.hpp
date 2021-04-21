#pragma once

#include <vector>
#include <optional>
#include <atomic>
#include <unordered_map>
#include <ranges>

#include "../DaxaCore.hpp"
#include "../threading/Jobs.hpp"
#include "../threading/OwningMutex.hpp"

#include "Vulkan.hpp"
#include "Image.hpp"

namespace daxa {
	class ImageManager {
		inline static const u32 SLOT_SURVIVAL_FRAMES{ 100 };
	public:
		ImageManager(vk::Device device = VulkanContext::device, u16 maxImages = (1024-1));

		struct ImageSlot {
			mutable OwningMutex<Image> imageMut;
			mutable std::atomic_uint32_t refCount{ 0 };
			uint32_t leftSurvivalFrames{ 0 };

			bool holdsImage() const {
				return refCount != 0 && leftSurvivalFrames != 0;
			}

			operator bool() const { return holdsImage(); }
		};

		class Handle;

		class Handle {
		public:
			Handle() = default;
			Handle(const Handle& other);
			Handle(Handle&& other);
			~Handle();

			bool operator==(const Handle&) const = default;
			bool operator!=(const Handle&) const = default;
			Handle& operator=(Handle&&) = default;
			Handle& operator=(const Handle&) = default;

			ReadWriteLock<Image> get();
			ReadOnlyLock<Image> getConst() const;
			OwningMutex<Image>& getMtx();

			u16 getIndex() const { return index; }
		private:
			friend class ImageManager;
			friend class WeakHandle;

			Handle(ImageManager* manager, u16 index);

			ImageManager* manager{ nullptr };
			u16 index{ static_cast<u16>(~0) };
		};

		Handle getHandle(const std::string& alias);

		Handle getHandle(u16 index);

		class DestroyJob : public IJob {
		public:
			DestroyJob(OwningMutex<ImageManager>* managerMtx) : managerMtx{ managerMtx } {}
			void execute() override;
		private:
			OwningMutex<ImageManager>* managerMtx{ nullptr };
		};

		class CreateJob : public IJob {
		public:
			CreateJob(OwningMutex<ImageManager>* managerMtx) : managerMtx{ managerMtx } {}
			void execute() override;
		private:
			OwningMutex<ImageManager>* managerMtx{ nullptr };
		};

		void update();

		std::vector<vk::Image> getImages() const;

	private:
		const u16 MAX_IMAGE_SLOTS;
		vk::Device device;
		vkh::CommandBufferAllocator cmdPool{ VulkanContext::device, vk::CommandPoolCreateInfo{.queueFamilyIndex = VulkanContext::mainTransferQueueFamiltyIndex } };
		vkh::Pool<vk::Fence> fencePool {
			[=]() { return device.createFence(vk::FenceCreateInfo{}); },
			[=](vk::Fence fence) { device.destroyFence(fence); },
			[=](vk::Fence fence) { device.resetFences(fence); }
		};

		std::unordered_map<std::string, u16> aliasToSlot;
		std::unordered_map<u16, std::string> slotToAlias;

		std::vector<u16>		freeImageSlots;
		std::vector<ImageSlot>	imageSlots;

		std::vector<std::pair<std::string, u16>> createQ;
		std::vector<u16> destroyQ;
	};

	using ImageHandle = ImageManager::Handle;
}
