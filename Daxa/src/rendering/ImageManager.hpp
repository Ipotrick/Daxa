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
		inline static const u32 SLOT_SURVIVAL_FRAMES{ 2 };
	public:
		ImageManager(vk::Device device = vkh_old::device) : device{device} {}

		struct ImageSlot {
			mutable OwningMutex<Image> imageMut;
			mutable std::atomic_uint32_t refCount{ 0 };
			mutable std::atomic_uint32_t framesSinceZeroRefs{ 0 };
		};

		class Handle;

		class WeakHandle {
		public:
			WeakHandle() = default;

			bool operator==(const WeakHandle&) const = default;
			bool operator!=(const WeakHandle&) const = default;

			ReadWriteLock<Image> get();
			ReadOnlyLock<Image> getConst() const;
			OwningMutex<Image>& getMtx();
			Handle getStrongHandle() const;
		private:
			friend class Handle;
			friend class ImageManager;

			WeakHandle(ImageSlot* imgSlot);

			ImageSlot* imgSlot{ nullptr };
		};

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
			WeakHandle getWeakHandle() const;
		private:
			friend class ImageManager;
			friend class WeakHandle;

			Handle(ImageSlot* slot);

			ImageSlot* imgSlot{ nullptr };
		};

		Handle getHandle(const std::string& alias);

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

	private:
		vk::Device device;
		vkh::CommandPool cmdPool{ vkh_old::device, vkh_old::mainTransferQueueFamiltyIndex };
		vkh::Pool<vk::Fence> fencePool {
			[=]() { return device.createFence(vk::FenceCreateInfo{}); },
			[=](vk::Fence fence) { device.destroyFence(fence); },
			[=](vk::Fence fence) { device.resetFences(fence); }
		};

		std::unordered_map<std::string, ImageSlot*> aliasToSlot;

		std::vector<std::unique_ptr<ImageSlot>> imageSlots;

		std::vector<std::pair<std::string,ImageSlot*>> createQ;
		std::vector<ImageSlot*> destroyQ;
	};

	using ImageHandle = ImageManager::Handle;
}
