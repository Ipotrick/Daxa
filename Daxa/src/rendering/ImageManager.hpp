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
#include "vulkanhelper/Image.hpp"

// TODO have chage like behaviour:
// TODO add loadfactor requirement (erase elements when the load gets too big)
// TODO add time to destruction (things that have a zero ref for longer are getting destroyed earlier)

namespace daxa {
	class ImageManager {
	public:
		ImageManager(vk::Device device = vkh::device) : device{device} {}

		struct ImageSlot {
			ImageSlot()
			{
				std::cout << "created image slot\n";
			}

			mutable OwningMutex<vkh::Image> imageMut;
			mutable std::atomic_uint32_t refCount{ 0 };
		};

		struct Handle {
			Handle() = default;

			Handle(ImageSlot* slot);

			Handle(const Handle& other);

			Handle(Handle&& other);

			~Handle();

			ReadWriteLock<vkh::Image> get();

			ReadOnlyLock<vkh::Image> getConst();

			bool operator==(const Handle&) const = default;
			bool operator!=(const Handle&) const = default;
		private:
			friend class ImageManager;
			ImageSlot* imgSlot{ nullptr };
		};

		Handle getHandle(const std::string& alias)
		{
			u32 index{ 0xFFFFFFFF };
			if (aliasToIndex.contains(alias)) {
				index = aliasToIndex[alias];
			} 
			else{
				if (freeIndices.empty()) {
					imageSlots.emplace_back(std::make_unique<ImageSlot>());
					index = static_cast<u32>(imageSlots.size() - 1);
				}
				else {
					index = freeIndices.back();
					freeIndices.pop_back();
				}
				aliasToIndex.insert({ alias,index });
			}
			createQ.push_back({ alias,index });
			return { imageSlots.at(index).get() };
		}

		class DestroyJob : public IJob {
		public:
			DestroyJob(OwningMutex<ImageManager>* managerMtx) : managerMtx{ managerMtx } {}
			void execute() override
			{
				auto manager = managerMtx->lock();
				for (auto [alias, index] : manager->destroyQ) {
					if (manager->imageSlots.at(index)->refCount == 0) {
						manager->aliasToIndex.erase(alias);
						*manager->imageSlots.at(index)->imageMut.lock() = vkh::Image{};
						manager->freeIndices.push_back(index);
					}
				}
				manager->destroyQ.clear();
			}
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

		void update()
		{
			for (uz i = 0; i < imageSlots.size(); ++i) {
				if (imageSlots[i]) {
					if (imageSlots[i]->refCount == 0) {
						destroyQ.emplace_back(indexToAlias[i], i);
					}
				}
			}
			cmdPool.flush();
		}

	private:
		vk::Device device;
		vkh::CommandPool cmdPool{ vkh::mainTransferQueueFamiltyIndex };
		vkh::Pool<vk::Fence> fencePool {
			[=]() { return device.createFence(vk::FenceCreateInfo{}); },
			[=](vk::Fence fence) { device.destroyFence(fence); },
			[=](vk::Fence fence) { device.resetFences(fence); }
		};

		std::unordered_map<std::string, u32> aliasToIndex;
		std::unordered_map<u32, std::string> indexToAlias;

		std::vector<u32> freeIndices;
		std::vector<std::unique_ptr<ImageSlot>> imageSlots;

		std::vector<std::pair<std::string, u32>> createQ;
		std::vector<std::pair<std::string, u32>> destroyQ;
	};

	using ImageHandle = ImageManager::Handle;
}
