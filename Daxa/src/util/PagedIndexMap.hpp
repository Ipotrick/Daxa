#pragma once

#include <vector>
#include <array>
#include <memory>
#include <cassert>
#include <optional>

#include "../DaxaCore.hpp"

namespace daxa {
	template<typename T, size_t PAGE_SIZE_EXPONENT = 6>
	class PagedIndexMap {
	public:
		PagedIndexMap() = default;
		PagedIndexMap(PagedIndexMap&& rhs) = default;
		PagedIndexMap(const PagedIndexMap& rhs) = delete;

		void insert(const T& t, const u32 index) 	{
			assert(!contains(index));
			assurePage(index);
			std::unique_ptr<Page>& pagePtr = pages[page(index)];
			const u32 offsetIndex = offset(index);
			pagePtr->usedSlotsCount += 1;
			pagePtr->slots[offsetIndex] = t;
		}

		void insert(T&& t, const u32 index) 	{
			assert(!contains(index));
			assurePage(index);
			std::unique_ptr<Page>& pagePtr = pages[page(index)];
			const u32 offsetIndex = offset(index);
			pagePtr->usedSlotsCount += 1;
			pagePtr->slots[offsetIndex] = std::move(t);
		}

		bool contains(u32 index) const 	{
			const u32 pageIndex = page(index);
			return
				pageIndex < pages.size() &&
				pages[pageIndex] != nullptr &&
				pages[pageIndex]->slots[offset(index)].has_value();
		}

		const T& get(u32 index) const 	{
			assert(contains(index));
			return pages[page(index)]->slots[offset(index)].value();
		}

		T& get(u32 index) 	{
			assert(contains(index));
			return pages[page(index)]->slots[offset(index)].value();
		}

		// TODO(Patrick) FIX THIS:
		const T& get_or(u32 index, T& other) const 	{
			return contains(index) ? get(index) : other;
		}

		void erase(u32 index) 	{
			assert(contains(index));
			std::unique_ptr<Page>& pagePtr = pages[page(index)];
			const u32 offsetIndex = offset(index);
			assert(pagePtr->slots[offsetIndex].has_value());
			pagePtr->usedSlotsCount -= 1;
			pagePtr->slots[offsetIndex] = std::nullopt;
			if (pagePtr->usedSlotsCount == 0) {
				pagePtr.reset();
			}
		}

		size_t memConsumtion() const 	{
			size_t memSize{ 0 };
			for (auto& pagePtr : pages) {
				memSize += sizeof(std::unique_ptr<T>);
				if (pagePtr) {
					memSize += sizeof(Page);
				}
			}
			return memSize;
		}

		void clear() 	{
			pages.clear();
		}
	private:
		static const int PAGE_BITS{ PAGE_SIZE_EXPONENT };
		static const int PAGE_SIZE{ 1 << PAGE_BITS };
		static const int OFFSET_MASK{ ~(-1 << PAGE_BITS) };

		struct Page {
			std::array<std::optional<T>, PAGE_SIZE> slots;
			u32 usedSlotsCount{ 0 };
		};

		static int page(u32 entity) 	{
			return entity >> PAGE_BITS;
		}

		static int offset(u32 entity) 	{
			return entity & OFFSET_MASK;
		}

		void assurePage(const u32 index) 	{
			if (page(index) + 1 > pages.size()) {
				pages.resize(page(index) + 1);
			}
			if (!pages[page(index)]) {
				pages[page(index)] = std::make_unique<Page>();
			}
		}

		std::vector<std::unique_ptr<Page>> pages;
	};
}