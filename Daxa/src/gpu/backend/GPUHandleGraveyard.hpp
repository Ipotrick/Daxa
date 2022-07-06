#pragma once

#include "../../DaxaCore.hpp"
#include "Backend.hpp"

#include <memory>
#include <mutex>
#include <vector>

#include "../GPUHandles.hpp"
#include "GPURessources.hpp"

namespace daxa {
	struct Zombie {
		GPUHandleType type;
		GPUHandle handle;
	};

	class ZombieList {
	public:
		void add(Zombie zombie) {
			zombies.push_back(zombie);
		}

		void clear(VkDevice device, VmaAllocator allocator, GPURessourceTable& table) {
			for (auto zombie : zombies) {
				switch (zombie.type) {
					case GPUHandleType::Buffer:
					{
						BufferHandle bufferHandle{ zombie.handle.index, zombie.handle.version };
						table.buffers.get(bufferHandle).zombieReferences -= 1;
						if (table.buffers.get(bufferHandle).zombieReferences == 0) {
							destroyBufferAndRemoveFromTable(device, allocator, table, bufferHandle);
						}
						break;
					}
					case GPUHandleType::ImageView:
					{
						ImageViewHandle imageViewHandle{ zombie.handle.index, zombie.handle.version };
						table.imageViews.get(imageViewHandle).zombieReferences -= 1;
						if (table.imageViews.get(imageViewHandle).zombieReferences == 0) {
							destroyImageViewAndRemoveFromTable(device, table, imageViewHandle);
						}
						break;
					}
					case GPUHandleType::Sampler:
					{
						SamplerHandle samplerHandle{ zombie.handle.index, zombie.handle.version };
						table.samplers.get(samplerHandle).zombieReferences -= 1;
						if (table.samplers.get(samplerHandle).zombieReferences == 0) {
							destroySamplerAndRemoveFromTable(device, table, samplerHandle);
						}
						break;
					}
				}
			}
			zombies.clear();
		}
	private:
		std::vector<Zombie> zombies = {};
	};

	struct GPUHandleGraveyard {
	public:
		void zombifyBuffer(GPURessourceTable& table, BufferHandle handle) {
			for (auto& zombieList : activeZombieLists) {
				zombieList->add(Zombie{
					.type = GPUHandleType::Buffer,
					.handle = handle
				});
				table.buffers.get(handle).zombieReferences += 1;
			}
		}

		void zombifyImageView(GPURessourceTable& table, ImageViewHandle handle) {
			for (auto& zombieList : activeZombieLists) {
				zombieList->add(Zombie{
					.type = GPUHandleType::ImageView,
					.handle = handle
				});
				table.imageViews.get(handle).zombieReferences += 1;
			}
		}

		void zombifySampler(GPURessourceTable& table, SamplerHandle handle) {
			for (auto& zombieList : activeZombieLists) {
				zombieList->add(Zombie{
					.type = GPUHandleType::Sampler,
					.handle = handle
				});
				table.samplers.get(handle).zombieReferences += 1;
			}
		}

		void activateZombieList(std::shared_ptr<ZombieList> list) {
			activeZombieLists.push_back(std::move(list));
		}

		void deactivateZombieList(std::shared_ptr<ZombieList> list) {
			auto iter = std::find_if(activeZombieLists.begin(), activeZombieLists.end(), [&](std::shared_ptr<ZombieList>& other){ return other.get() == list.get(); });
			if (iter != activeZombieLists.end()) {
				activeZombieLists.erase(iter);
			}
		}
	private:
		std::vector<std::shared_ptr<ZombieList>> activeZombieLists = {};
	};
}