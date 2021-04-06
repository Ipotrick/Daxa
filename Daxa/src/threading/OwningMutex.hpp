#pragma once

#include <shared_mutex>
#include <optional>

#include "../DaxaCore.hpp"

namespace daxa {

	template<typename T>
	class ReadWriteLock {
	public:
		ReadWriteLock(std::unique_lock<std::shared_mutex>&& lck, T* data) :
			lock{ std::move(lck) }, data{ data }
		{ }

		T& operator*()
		{
			DAXA_ASSERT(data);
			return *data;
		}
		const T& operator*() const
		{
			DAXA_ASSERT(data);
			return *data;
		}
		T* operator->()
		{
			DAXA_ASSERT(data);
			return data;
		}
		const T* operator->() const
		{
			DAXA_ASSERT(data);
			return data;
		}

		void unlock()
		{
			lock.unlock();
			data = nullptr;
		}

	private:
		std::unique_lock<std::shared_mutex> lock;
		T* data{ nullptr };
	};

	template<typename T>
	class ReadOnlyLock {
	public:
		ReadOnlyLock(std::shared_lock<std::shared_mutex>&& lck, T const* data) :
			lock{ std::move(lck) }, data{ data }
		{ }

		const T& operator*()
		{
			DAXA_ASSERT(data);
			return *data;
		}
		const T* operator->() const
		{
			DAXA_ASSERT(data);
			return data;
		}

		void unlock()
		{
			lock.unlock();
			data = nullptr;
		}

	private:
		std::shared_lock<std::shared_mutex> lock;
		T const* data{ nullptr };
	};

	template<typename T>
	class OwningMutex {
		static_assert(std::is_fundamental<T>::value == false, "please use an atomic for fundamental types!");
	public:
		OwningMutex() = default;

		template<typename... Args>
		OwningMutex(Args&&... args) :
			data{ std::forward<Args>(args)... }
		{ }

		OwningMutex(T&& e) : 
			data{ std::move(e) }
		{ }
		
		OwningMutex(const T& e) :
			data{ e }
		{ }

		ReadWriteLock<T> lock()
		{
			return ReadWriteLock<T>(std::unique_lock<std::shared_mutex>(mtx), &data);
		}

		std::optional<ReadWriteLock<T>> tryLock()
		{
			std::unique_lock<std::shared_mutex> lock(mtx, std::defer_lock);
			if (lock.try_lock()) {
				return ReadWriteLock<T>(std::move(lock), &data);
			}
			else {
				return {};
			}
		}

		ReadOnlyLock<T> lockReadOnly() const
		{
			return ReadOnlyLock<T>(std::shared_lock<std::shared_mutex>(mtx), &data);
		}

		std::optional<ReadOnlyLock<T>> tryLockReadOnly()
		{
			std::shared_lock<std::shared_mutex> lock(mtx, std::defer_lock);
			if (lock.try_lock()) {
				return ReadOnlyLock<T>(std::move(lock), &data);
			}
			else {
				return {};
			}
		}

	private:
		T data;
		mutable std::shared_mutex mtx;
	};
}
