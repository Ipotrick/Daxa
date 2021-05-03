#pragma once

#include <vector>
#include <array>
#include <memory>
#include <iterator>

#include "../DaxaCore.hpp"

namespace daxa {
	template<typename T>
	class StableVector {
	public:

		void pushBack(T&& element) {
			uz pageIndex = page(elementsSize); 
			assurePages(pageIndex);
			(*pages[pageIndex])[offset(elementsSize)] = std::move(element);
			elementsSize++;
		}
		
		void pushBack(const T& element) {
			uz pageIndex = page(elementsSize);
			assurePages(pageIndex);
			(*pages[pageIndex])[offset(elementsSize)] = element;
			elementsSize++;
		}

		template<typename ... Args>
		void emplaceBack(Args && ... args) {
			uz pageIndex = page(elementsSize);
			assurePages(pageIndex);
			new (&(*pages[pageIndex])[offset(elementsSize)]) T(std::forward<Args>(args)...);
			elementsSize++;
		}

		void popBack() {
			uz lastElementIndex = elementsSize - 1;
			if constexpr (std::is_trivially_destructible_v<T>) {
				(*pages[page(lastElementIndex)])[offset(lastElementIndex)].~T();
			}
			elementsSize--;
		}

		const T& operator[](uz index) const {
			return (*pages[page(index)])[offset(index)];
		}

		T& operator[](uz index) {
			return (*pages[page(index)])[offset(index)];
		}

		const T& at(uz index) const {
			return (*pages.at(page(index)))[offset(index)];
		}

		T& at(uz index) {
			return (*pages.at(page(index)))[offset(index)];
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

		void resize(uz newElementsSize) {
			if constexpr (!std::is_trivially_destructible_v<T>) {
				for (uz i = newElementsSize; i < elementsSize; i++) {
					operator[](i).~T();
				}
			}
			assurePages(page(newElementsSize - 1ull));
			elementsSize = newElementsSize;
		}

		class iterator {
		public:
			using difference_type = uz;
			using value_type = T;
			using pointer = T*;
			using reference = T&;
			using iterator_category = std::random_access_iterator_tag;
			iterator& operator++() { 
				++index;
				DAXA_ASSERT(this->index >= 0 && this->index <= master->size());
				return *this;
			}
			iterator operator++(int) { 
				iterator retval = *this; 
				++(*this); 
				return retval;
			}
			iterator& operator--() {
				--index;
				DAXA_ASSERT(this->index >= 0 && this->index <= master->size());
				return *this;
			}
			iterator operator--(int) {
				iterator retval = *this;
				--(*this);
				return retval;
			}
			iterator& operator+(iterator const& other) {
				this->index += other.index;
				DAXA_ASSERT(this->index >= 0 && this->index <= master->size());
				return this;
			}
			iterator& operator+(uz const& other) {
				this->index += other;
				DAXA_ASSERT(this->index >= 0 && this->index <= master->size());
				return this;
			}
			iterator& operator-(iterator const& other) {
				this->index -= other.index;
				DAXA_ASSERT(this->index >= 0 && this->index <= master->size());
				return this;
			}
			iterator& operator-(uz const& other) {
				this->index -= other;
				DAXA_ASSERT(this->index >= 0 && this->index <= master->size());
				return this;
			}
			bool operator==(iterator other) const { 
				return this->index == other.index && this->master == other.master;
			}
			bool operator!=(iterator other) const {
				return !(*this == other);
			}
			T& operator*() {
				return (*master)[index];
			}
			T* operator->() {
				return &(*master)[index];
			}
			const T& operator*() const {
				return (*master)[index];
			}
			const T* operator->() const {
				return &(*master)[index];
			}
		private:
			friend class StableVector;
			explicit iterator(StableVector<T>* master, uz index = 0) : master{ master }, index{index} {}
			StableVector<T>* master{nullptr};
			uz index{ 0 };
		};

		iterator begin() {
			return iterator(this, 0);
		}

		iterator end() {
			return iterator(this, elementsSize);
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
			const auto oldSize = pages.size();
			if (pageIndex >= pages.size()) {
				pages.resize(pageIndex + 1);
			}
			for (auto i = oldSize; i < pages.size(); i++) {
				if (!pages[i]) {
					pages[i] = std::make_unique<Page>();
				}
			}
		}

		std::vector<std::unique_ptr<Page>> pages;
		uz elementsSize{ 0 };
	};
}
