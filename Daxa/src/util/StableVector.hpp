#pragma once

#include <vector>
#include <array>
#include <memory>

#include "../DaxaCore.hpp"

namespace daxa {
	template<typename T>
	class StableVector {
	public:

		void pushBack(T&& element) {
			uz pageIndex = page(elementsSize); 
			assurePages(pageIndex);
			pages[pageIndex][offset(elementsSize)] = std::move(element);
			elementsSize++;
		}
		
		void pushBack(const T& element) {
			uz pageIndex = page(elementsSize);
			assurePages(pageIndex);
			pages[pageIndex][offset(elementsSize)] = element;
			elementsSize++;
		}

		template<typename ... Args>
		void emplaceBack(Args && ... args) {
			uz pageIndex = page(elementsSize);
			assurePages(pageIndex);
			new (&pages[pageIndex][offset(elementsSize)]) T(std::move(args));
			elementsSize++;
		}

		void popBack() {
			uz lastElementIndex = elementsSize - 1;
			if constexpr (std::is_trivially_destructible_v<T>) {
				pages[page(lastElementIndex)][offset(lastElementIndex)].~T();
			}
			elementsSize--;
		}

		const T& operator[](uz index) const {
			return pages[page(index)][offset(index)];
		}

		T& operator[](uz index) {
			return pages[page(index)][offset(index)];
		}

		const T& at(uz index) const {
			return pages.at(page(index))[offset(index)];
		}

		T& at(uz index) {
			return pages.at(page(index))[offset(index)];
		}

		bool empty() const {
			return elementsSize > 0;
		}

		uz size() const {
			return elementsSize;
		}

		uz capacity() const {
			return pages.size() * PAGE_SIZE;
		}

		void reserve(uz capacity) const {
			assurePages(page(capacity+1));
		}

		void clear() {
			if constexpr (!std::is_trivially_destructible_v<T>) {
				for (uz i = 0; i < elementsSize; i++) {
					operator[](i).~T();
				}
			}
			elementsSize = 0;
		}

		const T& front() const {
			return operator[](0);
		}

		T& front() {
			return operator[](0);
		}

		const T& back() const {
			return operator[](elementsSize-1);
		}

		T& back() {
			return operator[](elementsSize - 1);
		}

	private:
		static constexpr uz PAGE_EXPONENT = 10;
		static constexpr uz PAGE_SIZE = 1 << PAGE_EXPONENT;
		static constexpr uz OFFSET_MASK = ~(~static_cast<uz>(0) << PAGE_EXPONENT);
		using Page = std::array<T, PAGE_SIZE>;
		static constexpr uz page(uz index) {
			return index >> PAGE_EXPONENT;
		}
		static constexpr uz offset(uz index) {
			return index & OFFSET_MASK;
		}
		void assurePages(uz pageIndex) {
			if (pageIndex >= pages.size()) {
				pages.resize(pageIndex + 1, std::make_unique<Page>());
			}
		}

		std::vector<std::unique_ptr<Page>> pages;
		uz elementsSize{ 0 };
	};
}
