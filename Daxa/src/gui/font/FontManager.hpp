#pragma once

#include <vector>
#include <optional>
#include <atomic>
#include <unordered_map>
#include <ranges>

#include "../../DaxaCore.hpp"
#include "../../threading/Jobs.hpp"
#include "../../threading/OwningMutex.hpp"
#include "../../math/Vec2.hpp"
#include "Font.hpp"

namespace daxa {
	class FontManager {
		inline static const u32 SLOT_SURVIVAL_FRAMES{ 100 };
	public:
		FontManager(u16 maxSlots = (1024 - 1));

		struct ImageSlot {
			mutable OwningMutex<Font> mtx;
			mutable std::atomic_uint32_t refCount{ 0 };
			uint32_t leftSurvivalFrames{ 0 };

			bool holdsValue() const {
				return refCount != 0 && leftSurvivalFrames != 0;
			}

			operator bool() const { return holdsValue(); }
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

			ReadWriteLock<Font> get();
			ReadOnlyLock<Font> getConst() const;
			OwningMutex<Font>& getMtx();

			u16 getIndex() const { return index; }

			bool holdsValue() const {
				return manager != nullptr;
			}

			operator bool() const {
				return holdsValue();
			}
		private:
			friend class FontManager;
			friend class WeakHandle;

			Handle(FontManager* manager, u16 index);

			FontManager* manager{ nullptr };
			u16 index{ static_cast<u16>(~0) };
		};

		Handle getHandle(const std::string& alias);

		Handle getHandle(u16 index);

		class DestroyJob : public IJob {
		public:
			DestroyJob(OwningMutex<FontManager>* managerMtx) : managerMtx{ managerMtx } {}
			bool execute() override;
		private:
			OwningMutex<FontManager>* managerMtx{ nullptr };
		};

		class CreateJob : public IJob {
		public:
			CreateJob(OwningMutex<FontManager>* managerMtx) : managerMtx{ managerMtx } {}
			bool execute() override;
		private:
			OwningMutex<FontManager>* managerMtx{ nullptr };
		};

		void update();

	private:
		const u16 MAX_SLOTS;

		std::unordered_map<std::string, u16> aliasToSlot;
		std::unordered_map<u16, std::string> slotToAlias;

		std::vector<u16>		freeSlots;
		std::vector<ImageSlot>	slots;

		std::vector<std::pair<std::string, u16>> createQ;
	};

	using FontHandle = FontManager::Handle;
}
