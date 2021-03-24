#pragma once

#include <mutex>

#include "../DaxaCore.hpp"

namespace daxa {

	template<typename T>
	class OwningLock {
		static_assert(std::is_fundamental<T>::value == false, "please use an atomic for fundamental types!");
	public:
		OwningLock(T* data, std::mutex& mtx) :
			data{data}, lock{mtx}
		{ }

		T& get()
		{
			DAXA_ASSERT(data);
			return *data;
		}
		
		const T& get() const
		{
			DAXA_ASSERT(data);
			return *data;
		}

		void unlock()
		{
			lock.unlock();
			data = nullptr;
		}

	private:
		std::unique_lock<std::mutex> lock;
		T* data{ nullptr };
	};

	template<typename T>
	class OwningMutex {
		static_assert(std::is_fundamental<T>::value == false, "please use an atomic for fundamental types!");
	public:
		OwningMutex() = default;

		OwningMutex(T&& e) : 
			data{std::move(e)}
		{ }
		
		OwningMutex(const T& e) :
			data{ e }
		{ }

		OwningLock<T> lock()
		{
			return OwningLock<T>(&data, mtx);
		}
	private:
		T data;
		std::mutex mtx;
	};
}
