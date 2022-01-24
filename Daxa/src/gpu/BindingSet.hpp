#pragma once

#include "../DaxaCore.hpp"

#include <vector>
#include <array>
#include <optional>
#include <variant>
#include <span>
#include <array>
#include <unordered_map>
#include <mutex>

#include "vulkan/vulkan.h"

#include "DeviceBackend.hpp"
#include "Handle.hpp"
#include "Image.hpp"
#include "Buffer.hpp"
#include "Sampler.hpp"

namespace daxa {
	namespace gpu {

		constexpr inline size_t MAX_BINDINGS_PER_SET = 16;
		char const* const MORE_THAN_MAX_BINDINGS_MESSAGE = "a binding set can only have up to 16 bindings";

		class BindingSet;

		namespace {
			struct BindingSetAllocatorBindingiSetPool {
				VkDescriptorPool 							pool 			= VK_NULL_HANDLE;
				size_t 										allocatedSets 	= 0;
				std::vector<std::shared_ptr<BindingSet>> 	zombies 		= {};
				std::mutex 									mut 			= {};
			};
		}

		struct BindingSetDescription {
			std::span<VkDescriptorSetLayoutBinding> getBIndingsSpan() { return std::span<VkDescriptorSetLayoutBinding>(bindings.data(), bindingsCount); }
			std::span<VkDescriptorSetLayoutBinding const> getBIndingsSpan() const { return std::span<VkDescriptorSetLayoutBinding const>(bindings.data(), bindingsCount); }

			bool operator==(BindingSetDescription const& other) const {
				return std::memcmp(this, &other, sizeof(BindingSetDescription)) == 0;
			}

			std::array<VkDescriptorSetLayoutBinding, MAX_BINDINGS_PER_SET> 	bindings 		= {};
			size_t 															bindingsCount 	= 0;
		};

		struct BindingSetDesciptionHasher {
			size_t operator()(BindingSetDescription const& description) const
			{
				size_t hash = description.bindingsCount;
				for (int i = 0; i < MAX_BINDINGS_PER_SET; i++) {
					hash ^= description.bindings[i].binding;
					hash <<= 3;
					hash ^= description.bindings[i].descriptorCount;
					hash <<= 3;
					hash ^= description.bindings[i].descriptorType;
					hash <<= 3;
					hash ^= description.bindings[i].stageFlags;
					hash <<= 3;
				}
				return hash;
			}
		};

		class BindingSetInfo {
		public:
			BindingSetInfo(std::shared_ptr<DeviceBackend> deviceBackend, BindingSetDescription const& description);
			BindingSetInfo(BindingSetInfo const&) 			= delete;
			BindingSetInfo operator=(BindingSetInfo const&) = delete;
			BindingSetInfo(BindingSetInfo&&) noexcept;
			BindingSetInfo& operator=(BindingSetInfo&&) noexcept;
			~BindingSetInfo();

			BindingSetDescription const& getDescription() const { return description; }
			VkDescriptorSetLayout const& getVkDescriptorSetLayout() const { return layout; }
		private:
			void cleanup();

			std::shared_ptr<DeviceBackend> 	deviceBackend 	= {};
			BindingSetDescription 			description 	= {};
			VkDescriptorSetLayout 			layout			= {};
			size_t 							descriptorCount = {};
		};

		class BindingSet {
		public:
			BindingSet(std::shared_ptr<DeviceBackend> deviceBackend, VkDescriptorSet set, std::weak_ptr<BindingSetAllocatorBindingiSetPool> pool, std::shared_ptr<BindingSetInfo const> info);
			BindingSet(BindingSet const&)					= delete;
			BindingSet& operator=(BindingSet const&)		= delete;
			BindingSet(BindingSet&&) noexcept				= delete;
			BindingSet& operator=(BindingSet&&) noexcept 	= delete;

			void bindSamplers(u32 binding, std::span<SamplerHandle> samplers, u32 descriptorArrayOffset = 0);
			void bindSampler(u32 binding, SamplerHandle set, u32 dstArrayElement = 0);

			void bindBuffers(u32 binding, std::span<BufferHandle> buffers, u32 descriptorArrayOffset = 0);
			void bindBuffer(u32 binding, BufferHandle, u32 dstArrayElement = 0);

			void bindImages(u32 binding, std::span<std::pair<ImageHandle, VkImageLayout>> images, u32 descriptorArrayOffset = 0);
			void bindImage(u32 binding, ImageHandle image, VkImageLayout imgLayout, u32 dstArrayElement = 0);

			VkDescriptorSet getVkDescriptorSet() const { return set; }
			BindingSetInfo const& getInfo() { return *info; }
			std::shared_ptr<BindingSetInfo const> getInfoShared() { return info; }

			std::string const& getDebugName() const { return debugName; }
		private:
			friend class Device;
			friend class CommandList;
			friend class BindingSetAllocator;
			friend class CommandQueue;
			friend struct BindingSetHandleStaticFunctionOverride;

			void setDebugName(char const* debugName);

			std::shared_ptr<DeviceBackend> 						deviceBackend 	= {};
			VkDescriptorSet 									set 			= VK_NULL_HANDLE;
			std::shared_ptr<BindingSetInfo const>				info 			= {};
			std::weak_ptr<BindingSetAllocatorBindingiSetPool> 	pool 			= {};
			u32 												usesOnGPU 		= 0;
			std::string 										debugName		= {};
		};

		class BindingSetDescriptionCache {
		public:
			BindingSetDescriptionCache(std::shared_ptr<DeviceBackend> deviceBackend);
			BindingSetDescriptionCache(BindingSetDescriptionCache const&) 					= delete;
			BindingSetDescriptionCache& operator=(BindingSetDescriptionCache const&) 		= delete;
			BindingSetDescriptionCache(BindingSetDescriptionCache&&) noexcept 				= delete;
			BindingSetDescriptionCache& operator=(BindingSetDescriptionCache&&) noexcept 	= delete;

			BindingSetInfo const& getInfo(BindingSetDescription const& description);
			std::shared_ptr<BindingSetInfo const> getInfoShared(BindingSetDescription const& description);
		private:
			using MapT = std::unordered_map<BindingSetDescription, std::shared_ptr<BindingSetInfo>, BindingSetDesciptionHasher>;

			std::shared_ptr<DeviceBackend> 	deviceBackend 	= {};
			MapT							map 			= {};
		};

		struct BindingSetHandleStaticFunctionOverride {
			static void cleanup(std::shared_ptr<BindingSet>& value) {
				if (value && value.use_count() == 1) {
					auto pool = value->pool.lock();
					auto lock = std::unique_lock(pool->mut);
					pool->zombies.push_back(std::move(value));
				}
			}
		};

		class BindingSetHandle : public SharedHandle<BindingSet, BindingSetHandleStaticFunctionOverride>{};

		struct BindingSetAllocatorCreateInfo {
			std::shared_ptr<BindingSetInfo const> 	setInfo 	= {};
			size_t 									setPerPool 	= 64;
			char const* 							debugName 	= {};
		};

		/**
		* Must be externally Synchronized. given out BindingSets must not be externally synchorized.
		*/
		class BindingSetAllocator {
		public:
			BindingSetAllocator(std::shared_ptr<DeviceBackend> deviceBackend, BindingSetAllocatorCreateInfo const& ci);
			BindingSetAllocator() 											= default;
			BindingSetAllocator(BindingSetAllocator&&) noexcept 			= delete;
			BindingSetAllocator& operator=(BindingSetAllocator&&) noexcept 	= delete;
			BindingSetAllocator(BindingSetAllocator const&) 				= delete;
			BindingSetAllocator& operator=(BindingSetAllocator const&) 		= delete;
			~BindingSetAllocator();

			BindingSetHandle getSet(const char* debugName = {});
			std::string const& getDebugName() const { return debugName; }
		private:
			void initPoolSizes();
			BindingSetHandle getNewSet(std::shared_ptr<BindingSetAllocatorBindingiSetPool>& pool);
			std::shared_ptr<BindingSetAllocatorBindingiSetPool> getNewPool();

			std::shared_ptr<DeviceBackend> 										deviceBackend 	= {};
			size_t 																setsPerPool 	= 0;
			std::vector<VkDescriptorPoolSize> 									poolSizes 		= {};
			std::shared_ptr<BindingSetInfo const> 								setInfo 		= {};
			std::vector<std::shared_ptr<BindingSetAllocatorBindingiSetPool>> 	pools 			= {};
			std::string 														debugName 		= {};
			std::string 														poolNameBuffer	= {};
		};

		class BindingSetAllocatorHandle : public SharedHandle<BindingSetAllocator>{};
	}
}