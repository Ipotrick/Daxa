#pragma once

#include <shared_mutex>

#include "../DaxaCore.hpp"

namespace daxa {

	template<typename T>
	class ReadWriteLock {
	public:
		ReadWriteLock(std::shared_mutex& mtx, T* data) :
			lock{ mtx }, data{ data }
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
		ReadOnlyLock(std::shared_mutex& mtx, T const* data) :
			lock{ mtx }, data{ data }
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
			return ReadWriteLock<T>(mtx , &data);
		}
		ReadOnlyLock<T> lockReadOnly() const
		{
			return ReadOnlyLock<T>(mtx, &data);
		}
	private:
		T data;
		mutable std::shared_mutex mtx;
	};
}
