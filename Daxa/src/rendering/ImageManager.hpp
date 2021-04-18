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
		ImageManager(vk::Device device = VulkanContext::device) : device{device} {}

		struct ImageTuple {
			Image image;
			i32 tableIndex{ -1 };
		};

		struct ImageSlot {
			mutable OwningMutex<ImageTuple> imageMut;
			mutable std::atomic_uint32_t refCount{ 0 };
			mutable std::atomic_uint32_t framesSinceZeroRefs{ 0 };
		};

		class Handle;

		class WeakHandle {
		public:
			WeakHandle() = default;

			bool operator==(const WeakHandle&) const = default;
			bool operator!=(const WeakHandle&) const = default;

			ReadWriteLock<ImageTuple> get();
			ReadOnlyLock<ImageTuple> getConst() const;
			OwningMutex<ImageTuple>& getMtx();
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

			ReadWriteLock<ImageTuple> get();
			ReadOnlyLock<ImageTuple> getConst() const;
			OwningMutex<ImageTuple>& getMtx();
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

		void clearRegestry();

	private:
		vk::Device device;
		vkh::CommandBufferPool cmdPool{ VulkanContext::device, vk::CommandPoolCreateInfo{.queueFamilyIndex = VulkanContext::mainTransferQueueFamiltyIndex } };
		vkh::Pool<vk::Fence> fencePool {
			[=]() { return device.createFence(vk::FenceCreateInfo{}); },
			[=](vk::Fence fence) { device.destroyFence(fence); },
			[=](vk::Fence fence) { device.resetFences(fence); }
		};

		std::unordered_map<std::string, ImageSlot*> aliasToSlot;
		std::unordered_map<ImageSlot* , std::string> slotToAlias;

		std::vector<std::unique_ptr<ImageSlot>> imageSlots;

		std::vector<std::pair<std::string,ImageSlot*>> createQ;
		std::vector<ImageSlot*> destroyQ;
	};

	using ImageHandle = ImageManager::Handle;
}
